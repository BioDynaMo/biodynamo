// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_GRID_H_
#define CORE_GRID_H_

#include <assert.h>
#include <omp.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#ifdef LINUX
#include <parallel/algorithm>
#endif  // LINUX
#include <utility>
#include <vector>

#include <morton/morton.h>

#include "core/container/fixed_size_vector.h"
#include "core/container/inline_vector.h"
#include "core/container/math_array.h"
#include "core/container/parallel_resize_vector.h"
#include "core/container/sim_object_vector.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/util/log.h"
#include "core/util/spinlock.h"

namespace bdm {

/// FIFO data structure. Stores max N number of objects.
/// Adding more elements will overwrite the oldest ones.
template <typename T, uint64_t N>
class CircularBuffer {
 public:
  CircularBuffer() {
    for (uint64_t i = 0; i < N; i++) {
      data_[i] = T();
    }
  }

  void clear() {  // NOLINT
    position_ = 0;
    for (uint64_t i = 0; i < N; i++) {
      data_[i].clear();
    }
  }

  void push_back(const T& data) {  // NOLINT
    data_[position_] = data;
    Increment();
  }

  T& operator[](uint64_t idx) { return data_[(idx + position_) % N]; }

  const T& operator[](uint64_t idx) const {
    return data_[(idx + position_) % N];
  }

  /// Calling function `End` and afterwards `Increment` is equivalent to
  /// `push_back`, but avoids copying data
  T* End() { return &(data_[position_]); }

  void Increment() {
    position_++;
    position_ %= N;
  }

 private:
  T data_[N];
  uint64_t position_ = 0;
};

/// A class that represents Cartesian 3D grid
class Grid {
  // DisplacementOpCuda needs access to some Grid private members to reconstruct
  // the grid on GPU (same for DisplacementOpOpenCL)
  friend class DisplacementOpCuda;
  friend class DisplacementOpOpenCL;

 public:
  /// A single unit cube of the grid
  struct Box {
    Spinlock lock_;
    // std::atomic<bool> timestamp_;
    uint32_t timestamp_;
    /// start value of the linked list of simulation objects inside this box.
    /// Next element can be found at `successors_[start_]`
    SoHandle start_;
    /// length of the linked list (i.e. number of simulation objects)
    /// uint64_t, because sizeof(Box) = 16, for uint16_t and uint64_t
    uint16_t length_;

    Box() : timestamp_(0), start_(SoHandle()), length_(0) {}
    /// Copy Constructor required for boxes_.resize()
    /// Since box values will be overwritten afterwards it forwards to the
    /// default ctor
    Box(const Box& other) : Box() {}

    Box& operator=(const Box& other) {
      // start_ = other.start_.load(std::memory_order_relaxed);
      // length_ = other.length_.load(std::memory_order_relaxed);
      start_ = other.start_;
      length_ = other.length_;
      return *this;
    }

    bool IsEmpty(uint64_t grid_timestamp) const {
      return grid_timestamp != timestamp_;
    }

    /// @brief      Adds a simulation object to this box
    ///
    /// @param[in]  so       The object's identifier
    /// @param   AddObject   successors   The successors
    void AddObject(SoHandle so, SimObjectVector<SoHandle>* successors,
                   Grid* grid) {
      std::lock_guard<Spinlock> lock_guard(lock_);

      if (timestamp_ != grid->timestamp_) {
        timestamp_ = grid->timestamp_;
        length_ = 1;
        start_ = so;
      } else {
        length_++;
        (*successors)[so] = start_;
        start_ = so;
      }
    }

    /// An iterator that iterates over the cells in this box
    struct Iterator {
      Iterator(Grid* grid, const Box* box)
          : grid_(grid), current_value_(box->start_), countdown_(box->length_) {
        if (grid->timestamp_ != box->timestamp_) {
          countdown_ = 0;
        }
      }

      bool IsAtEnd() { return countdown_ <= 0; }

      Iterator& operator++() {
        countdown_--;
        if (countdown_ > 0) {
          current_value_ = grid_->successors_[current_value_];
        }
        return *this;
      }

      SoHandle operator*() const { return current_value_; }

      /// Pointer to the neighbor grid; for accessing the successor_ list
      Grid* grid_;
      /// The current simulation object to be considered
      SoHandle current_value_;
      /// The remain number of simulation objects to consider
      int countdown_ = 0;
    };

    Iterator begin() const {  // NOLINT
      return Iterator(Simulation::GetActive()->GetGrid(), this);
    }
  };

  /// An iterator that iterates over the boxes in this grid
  struct NeighborIterator {
    explicit NeighborIterator(
        const FixedSizeVector<const Box*, 27>& neighbor_boxes,
        uint64_t grid_timestamp)
        : neighbor_boxes_(neighbor_boxes),
          // start iterator from box 0
          box_iterator_(neighbor_boxes_[0]->begin()),
          grid_timestamp_(grid_timestamp) {
      // if first box is empty
      if (neighbor_boxes_[0]->IsEmpty(grid_timestamp)) {
        ForwardToNonEmptyBox(grid_timestamp);
      }
    }

    bool IsAtEnd() const { return is_end_; }

    SoHandle operator*() const { return *box_iterator_; }

    /// Version where empty neighbor boxes are allowed
    NeighborIterator& operator++() {
      ++box_iterator_;
      // if iterator of current box has come to an end, continue with next box
      if (box_iterator_.IsAtEnd()) {
        return ForwardToNonEmptyBox(grid_timestamp_);
      }
      return *this;
    }

   private:
    /// The 27 neighbor boxes that will be searched for simulation objects
    const FixedSizeVector<const Box*, 27>& neighbor_boxes_;
    /// The box that shall be considered to iterate over for finding simulation
    /// objects
    typename Box::Iterator box_iterator_;
    uint64_t grid_timestamp_;
    /// The id of the box to be considered (i.e. value between 0 - 26)
    uint16_t box_idx_ = 0;
    /// Flag to indicate that all the neighbor boxes have been searched through
    bool is_end_ = false;

    /// Forwards the iterator to the next non empty box and returns itself
    /// If there are no non empty boxes is_end_ is set to true
    NeighborIterator& ForwardToNonEmptyBox(uint64_t grid_timestamp) {
      // increment box id until non empty box has been found
      while (++box_idx_ < neighbor_boxes_.size()) {
        // box is empty or uninitialized (padding box) -> continue
        if (neighbor_boxes_[box_idx_]->IsEmpty(grid_timestamp)) {
          continue;
        }
        // a non-empty box has been found
        box_iterator_ = neighbor_boxes_[box_idx_]->begin();
        return *this;
      }
      // all remaining boxes have been empty; reached end
      is_end_ = true;
      return *this;
    }
  };

  /// Enum that determines the degree of adjacency in search neighbor boxes
  //  todo(ahmad): currently only kHigh is supported (hardcoded 26 several
  //  places)
  enum Adjacency {
    kLow,    /**< The closest 8  neighboring boxes */
    kMedium, /**< The closest 18  neighboring boxes */
    kHigh    /**< The closest 26  neighboring boxes */
  };

  Grid() {}

  Grid(Grid const&) = delete;
  void operator=(Grid const&) = delete;

  /// @brief      Initialize the grid with the given simulation objects
  /// @param[in]  adjacency    The adjacency (see #Adjacency)
  void Initialize(Adjacency adjacency = kHigh) {
    adjacency_ = adjacency;

    UpdateGrid();
    initialized_ = true;
  }

  virtual ~Grid() {}

  /// Clears the grid
  void ClearGrid() {
    box_length_ = 1;
    largest_object_size_ = 0;
    num_boxes_axis_ = {{0}};
    num_boxes_xy_ = 0;
    int32_t inf = std::numeric_limits<int32_t>::max();
    grid_dimensions_ = {inf, -inf, inf, -inf, inf, -inf};
    threshold_dimensions_ = {inf, -inf};
    successors_.clear();
    has_grown_ = false;
  }

  /// Updates the grid, as simulation objects may have moved, added or deleted
  void UpdateGrid() {
    auto* rm = Simulation::GetActive()->GetResourceManager();

    if (rm->GetNumSimObjects() != 0) {
      ClearGrid();
      timestamp_++;

      auto inf = Math::kInfinity;
      std::array<double, 6> tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
      CalculateGridDimensions(&tmp_dim);
      RoundOffGridDimensions(tmp_dim);

      auto los = ceil(largest_object_size_);
      assert(los > 0 &&
             "The largest object size was found to be 0. Please check if your "
             "cells are correctly initialized.");
      box_length_ = los;

      for (int i = 0; i < 3; i++) {
        int dimension_length =
            grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
        int r = dimension_length % box_length_;
        // If the grid is not perfectly divisible along each dimension by the
        // resolution, extend the grid so that it is
        if (r != 0) {
          // std::abs for the case that box_length_ > dimension_length
          grid_dimensions_[2 * i + 1] += (box_length_ - r);
        } else {
          // Else extend the grid dimension with one row, because the outmost
          // object lies exactly on the border
          grid_dimensions_[2 * i + 1] += box_length_;
        }
      }

      // Pad the grid to avoid out of bounds check when search neighbors
      for (int i = 0; i < 3; i++) {
        grid_dimensions_[2 * i] -= box_length_;
        grid_dimensions_[2 * i + 1] += box_length_;
      }

      // Calculate how many boxes fit along each dimension
      for (int i = 0; i < 3; i++) {
        int dimension_length =
            grid_dimensions_[2 * i + 1] - grid_dimensions_[2 * i];
        assert((dimension_length % box_length_ == 0) &&
               "The grid dimensions are not a multiple of its box length");
        num_boxes_axis_[i] = dimension_length / box_length_;
      }

      num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];
      auto total_num_boxes = num_boxes_xy_ * num_boxes_axis_[2];

      CheckGridGrowth();

      // resize boxes_
      if (boxes_.size() != total_num_boxes) {
        if (boxes_.capacity() < total_num_boxes) {
          boxes_.reserve(total_num_boxes * 2);
        }
        boxes_.resize(total_num_boxes);
      }

      successors_.reserve();

      // Assign simulation objects to boxes
      rm->ApplyOnAllElementsParallelDynamic(
          1000, [this](SimObject* sim_object, SoHandle soh) {
            const auto& position = sim_object->GetPosition();
            auto idx = this->GetBoxIndex(position);
            auto box = this->GetBoxPointer(idx);
            box->AddObject(soh, &successors_, this);
            sim_object->SetBoxIdx(idx);
          });
      auto* param = Simulation::GetActive()->GetParam();
      if (param->bound_space_) {
        int min = param->min_bound_;
        int max = param->max_bound_;
        threshold_dimensions_ = {min, max};
      }

      if (nb_mutex_builder_ != nullptr) {
        nb_mutex_builder_->Update();
      }
    } else {
      // There are no sim objects in this simulation
      auto* param = Simulation::GetActive()->GetParam();

      bool uninitialized = boxes_.size() == 0;
      if (uninitialized && param->bound_space_) {
        // Simulation has never had any simulation objects
        // Initialize grid dimensions with `Param::min_bound_` and
        // `Param::max_bound_`
        // This is required for the DiffusionGrid
        int min = param->min_bound_;
        int max = param->max_bound_;
        grid_dimensions_ = {min, max, min, max, min, max};
        threshold_dimensions_ = {min, max};
        has_grown_ = true;
      } else if (!uninitialized) {
        // all simulation objects have been removed in the last iteration
        // grid state remains the same, but we have to set has_grown_ to false
        // otherwise the DiffusionGrid will attempt to resize
        has_grown_ = false;
      } else {
        Log::Fatal(
            "Grid",
            "You tried to initialize an empty simulation without bound space. "
            "Therefore we cannot determine the size of the simulation space. "
            "Please add simulation objects, or set Param::bound_space_, "
            "Param::min_bound_, and Param::max_bound_.");
      }
    }
  }

  /// @brief      Calculates the squared euclidian distance between two points
  ///             in 3D
  ///
  /// @param[in]  pos1  Position of the first point
  /// @param[in]  pos2  Position of the second point
  ///
  /// @return     The distance between the two points
  ///
  inline double SquaredEuclideanDistance(const Double3& pos1,
                                         const Double3& pos2) const {
    const double dx = pos2[0] - pos1[0];
    const double dy = pos2[1] - pos1[1];
    const double dz = pos2[2] - pos1[2];
    return (dx * dx + dy * dy + dz * dz);
  }

  inline bool WithinSquaredEuclideanDistance(double squared_radius,
                                             const Double3& pos1,
                                             const Double3& pos2) const {
    const double dx = pos2[0] - pos1[0];
    const double dx2 = dx * dx;
    if (dx2 > squared_radius) {
      return false;
    }

    const double dy = pos2[1] - pos1[1];
    const double dy2_plus_dx2 = dy * dy + dx2;
    if (dy2_plus_dx2 > squared_radius) {
      return false;
    }

    const double dz = pos2[2] - pos1[2];
    const double distance = dz * dz + dy2_plus_dx2;
    return distance < squared_radius;
  }

  void UpdateBoxZOrder() {
    // iterate boxes in Z-order / morton order
    // TODO(lukas) this is a very quick attempt to test an idea
    // improve performance of this brute force solution
    zorder_sorted_boxes_.resize(boxes_.size());
#pragma omp parallel for collapse(3)
    for (uint32_t x = 0; x < num_boxes_axis_[0]; x++) {
      for (uint32_t y = 0; y < num_boxes_axis_[1]; y++) {
        for (uint32_t z = 0; z < num_boxes_axis_[2]; z++) {
          auto box_idx = GetBoxIndex(std::array<uint32_t, 3>{x, y, z});
          auto morton = libmorton::morton3D_64_encode(x, y, z);
          zorder_sorted_boxes_[box_idx] =
              std::pair<uint32_t, const Box*>{morton, &boxes_[box_idx]};
        }
      }
    }
#ifdef LINUX
    __gnu_parallel::sort(
        zorder_sorted_boxes_.begin(), zorder_sorted_boxes_.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
#else
    std::sort(
        zorder_sorted_boxes_.begin(), zorder_sorted_boxes_.end(),
        [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
#endif  // LINUX
  }

  /// This method iterates over all elements. Iteration is performed in
  /// Z-order of boxes. There is no particular order for elements inside a box.
  template <typename Lambda>
  void IterateZOrder(const Lambda& lambda) {
    UpdateBoxZOrder();
    for (uint64_t i = 0; i < zorder_sorted_boxes_.size(); i++) {
      auto it = zorder_sorted_boxes_[i].second->begin();
      while (!it.IsAtEnd()) {
        lambda(*it);
        ++it;
      }
    }
  }

  /// @brief      Applies the given lambda to each neighbor
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  void ForEachNeighbor(const std::function<void(const SimObject*)>& lambda,
                       const SimObject& query) const {
    auto idx = query.GetBoxIdx();

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    auto* rm = Simulation::GetActive()->GetResourceManager();

    NeighborIterator ni(neighbor_boxes, timestamp_);
    while (!ni.IsAtEnd()) {
      auto* sim_object = rm->GetSimObjectWithSoHandle(*ni);
      if (sim_object != &query) {
        lambda(sim_object);
      }
      ++ni;
    }
  }

  /// @brief      Applies the given lambda to each neighbor or the specified
  ///             simulation object.
  ///
  /// In simulation code do not use this function directly. Use the same
  /// function from the exeuction context (e.g. `InPlaceExecutionContext`)
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  ///
  void ForEachNeighbor(
      const std::function<void(const SimObject*, double)>& lambda,
      const SimObject& query) {
    const auto& position = query.GetPosition();
    auto idx = query.GetBoxIdx();

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    auto* rm = Simulation::GetActive()->GetResourceManager();

    NeighborIterator ni(neighbor_boxes, timestamp_);
    const unsigned batch_size = 64;
    uint64_t size = 0;
    SimObject* sim_objects[batch_size] __attribute__((aligned(64)));
    double x[batch_size] __attribute__((aligned(64)));
    double y[batch_size] __attribute__((aligned(64)));
    double z[batch_size] __attribute__((aligned(64)));
    double squared_distance[batch_size] __attribute__((aligned(64)));

    auto process_batch = [&]() {
#pragma omp simd
      for (uint64_t i = 0; i < size; ++i) {
        const double dx = x[i] - position[0];
        const double dy = y[i] - position[1];
        const double dz = z[i] - position[2];

        squared_distance[i] = dx * dx + dy * dy + dz * dz;
      }

      for (uint64_t i = 0; i < size; ++i) {
        lambda(sim_objects[i], squared_distance[i]);
      }
      size = 0;
    };

    while (!ni.IsAtEnd()) {
      auto soh = *ni;
      // increment iterator already here to hide memory latency
      ++ni;
      auto* sim_object = rm->GetSimObjectWithSoHandle(soh);
      if (sim_object != &query) {
        sim_objects[size] = sim_object;
        const auto& pos = sim_object->GetPosition();
        x[size] = pos[0];
        y[size] = pos[1];
        z[size] = pos[2];
        size++;
        if (size == batch_size) {
          process_batch();
        }
      }
    }
    process_batch();
  }

  /// @brief      Applies the given lambda to each neighbor or the specified
  ///             simulation object.
  ///
  /// In simulation code do not use this function directly. Use the same
  /// function from the exeuction context (e.g. `InPlaceExecutionContext`)
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  /// @param[in]  squared_radius  The search radius squared
  ///
  void ForEachNeighborWithinRadius(
      const std::function<void(const SimObject*)>& lambda,
      const SimObject& query, double squared_radius) {
    const auto& position = query.GetPosition();
    auto idx = query.GetBoxIdx();

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    auto* rm = Simulation::GetActive()->GetResourceManager();

    NeighborIterator ni(neighbor_boxes, timestamp_);
    while (!ni.IsAtEnd()) {
      // Do something with neighbor object
      auto* sim_object = rm->GetSimObjectWithSoHandle(*ni);
      if (sim_object != &query) {
        const auto& neighbor_position = sim_object->GetPosition();
        if (this->WithinSquaredEuclideanDistance(squared_radius, position,
                                                 neighbor_position)) {
          lambda(sim_object);
        }
      }
      ++ni;
    }
  }

  /// @brief      Return the box index in the one dimensional array of the box
  ///             that contains the position
  ///
  /// @param[in]  position  The position of the object
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const Double3& position) const {
    std::array<uint32_t, 3> box_coord;
    box_coord[0] = (floor(position[0]) - grid_dimensions_[0]) / box_length_;
    box_coord[1] = (floor(position[1]) - grid_dimensions_[2]) / box_length_;
    box_coord[2] = (floor(position[2]) - grid_dimensions_[4]) / box_length_;

    return GetBoxIndex(box_coord);
  }

  /// Gets the size of the largest object in the grid
  double GetLargestObjectSize() const { return largest_object_size_; }

  const std::array<int32_t, 6>& GetDimensions() const {
    return grid_dimensions_;
  }

  const std::array<int32_t, 2>& GetDimensionThresholds() const {
    return threshold_dimensions_;
  }

  uint64_t GetNumBoxes() const { return boxes_.size(); }

  uint32_t GetBoxLength() { return box_length_; }

  bool HasGrown() { return has_grown_; }

  std::array<uint32_t, 3> GetBoxCoordinates(size_t box_idx) const {
    std::array<uint32_t, 3> box_coord;
    box_coord[2] = box_idx / num_boxes_xy_;
    auto remainder = box_idx % num_boxes_xy_;
    box_coord[1] = remainder / num_boxes_axis_[0];
    box_coord[0] = remainder % num_boxes_axis_[0];
    return box_coord;
  }

  bool IsInitialized() { return initialized_; }

  /// @brief      Gets the information about the grid
  ///
  /// @param      box_length       The grid's box length
  /// @param      num_boxes_axis   The number boxes along each axis of the grid
  /// @param      grid_dimensions  The grid's dimensions
  ///
  /// @tparam     TUint32          A uint32 type (could also be cl_uint)
  /// @tparam     TInt32           A int32 type (could be cl_int)
  ///
  template <typename TUint32, typename TInt32>
  void GetGridInfo(TUint32* box_length, std::array<TUint32, 3>* num_boxes_axis,
                   std::array<TInt32, 3>* grid_dimensions) {
    box_length[0] = box_length_;
    (*num_boxes_axis)[0] = num_boxes_axis_[0];
    (*num_boxes_axis)[1] = num_boxes_axis_[1];
    (*num_boxes_axis)[2] = num_boxes_axis_[2];
    (*grid_dimensions)[0] = grid_dimensions_[0];
    (*grid_dimensions)[1] = grid_dimensions_[2];
    (*grid_dimensions)[2] = grid_dimensions_[4];
  }

  // NeighborMutex ---------------------------------------------------------

  /// This class ensures thread-safety for the InPlaceExecutionContext for the
  /// case
  /// that a simulation object modifies its neighbors.
  class NeighborMutexBuilder {
   public:
    /// The NeighborMutex class is a synchronization primitive that can be
    /// used to protect sim_objects data from being simultaneously accessed by
    /// multiple threads.
    class NeighborMutex {
     public:
      NeighborMutex(const FixedSizeVector<uint64_t, 27>& mutex_indices,
                    NeighborMutexBuilder* mutex_builder)
          : mutex_indices_(mutex_indices), mutex_builder_(mutex_builder) {
        // Deadlocks occur if mutliple threads try to acquire the same locks,
        // but in different order.
        // -> sort to avoid deadlocks - see lock ordering
        std::sort(mutex_indices_.begin(), mutex_indices_.end());
      }

      void lock() {  // NOLINT
        for (auto idx : mutex_indices_) {
          auto& mutex = mutex_builder_->mutexes_[idx].mutex_;
          // acquire lock (and spin if another thread is holding it)
          while (mutex.test_and_set(std::memory_order_acquire)) {
          }
        }
      }

      void unlock() {  // NOLINT
        for (auto idx : mutex_indices_) {
          auto& mutex = mutex_builder_->mutexes_[idx].mutex_;
          mutex.clear(std::memory_order_release);
        }
      }

     private:
      FixedSizeVector<uint64_t, 27> mutex_indices_;
      NeighborMutexBuilder* mutex_builder_;
    };

    /// Used to store mutexes in a vector.
    /// Always creates a new mutex (even for the copy constructor)
    struct MutexWrapper {
      MutexWrapper() {}
      MutexWrapper(const MutexWrapper&) {}
      std::atomic_flag mutex_ = ATOMIC_FLAG_INIT;
    };

    void Update() {
      auto* grid = Simulation::GetActive()->GetGrid();
      mutexes_.resize(grid->GetNumBoxes());
    }

    NeighborMutex GetMutex(uint64_t box_idx) {
      auto* grid = Simulation::GetActive()->GetGrid();
      FixedSizeVector<uint64_t, 27> box_indices;
      grid->GetMooreBoxIndices(&box_indices, box_idx);
      return NeighborMutex(box_indices, this);
    }

   private:
    /// one mutex for each box in `Grid::boxes_`
    std::vector<MutexWrapper> mutexes_;
  };

  /// Disable neighbor mutexes management. `GetNeighborMutexBuilder()` will
  /// return a nullptr.
  void DisableNeighborMutexes() { nb_mutex_builder_ = nullptr; }

  /// Returns the `NeighborMutexBuilder`. The client use it to create a
  /// `NeighborMutex`. If neighbor mutexes has been disabled by calling
  /// `DisableNeighborMutexes`, this function will return a nullptr.
  NeighborMutexBuilder* GetNeighborMutexBuilder() {
    return nb_mutex_builder_.get();
  }

 private:
  /// The vector containing all the boxes in the grid
  /// Using parallel resize vector to enable parallel initialization and thus
  /// better scalability.
  ParallelResizeVector<Box> boxes_;
  /// is incremented at each call to UpdateGrid
  /// This is used to decide if boxes should be reinitialized
  uint32_t timestamp_ = 0;
  /// Length of a Box
  uint32_t box_length_ = 1;
  /// Stores the number of boxes for each axis
  std::array<uint32_t, 3> num_boxes_axis_ = {{0}};
  /// Number of boxes in the xy plane (=num_boxes_axis_[0] * num_boxes_axis_[1])
  size_t num_boxes_xy_ = 0;
  /// Implements linked list - array index = key, value: next element
  ///
  ///     // Usage
  ///     SoHandle current_element = ...;
  ///     SoHandle next_element = successors_[current_element];
  SimObjectVector<SoHandle> successors_;
  /// Determines which boxes to search neighbors in (see enum Adjacency)
  Adjacency adjacency_;
  /// The size of the largest object in the simulation
  double largest_object_size_ = 0;
  /// Cube which contains all simulation objects
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  std::array<int32_t, 6> grid_dimensions_;
  /// Stores the min / max dimension value that need to be surpassed in order
  /// to trigger a diffusion grid change
  std::array<int32_t, 2> threshold_dimensions_;
  /// Flag to indicate that the grid dimensions have increased
  bool has_grown_ = false;
  /// Flag to indicate if the grid has been initialized or not
  bool initialized_ = false;
  /// stores pairs of <box morton code,  box pointer> sorted by morton code.
  ParallelResizeVector<std::pair<uint32_t, const Box*>> zorder_sorted_boxes_;

  /// Holds instance of NeighborMutexBuilder if it is enabled.
  /// If `DisableNeighborMutexes` has been called this member set to nullptr.
  std::unique_ptr<NeighborMutexBuilder> nb_mutex_builder_ =
      std::make_unique<NeighborMutexBuilder>();

  void CheckGridGrowth() {
    // Determine if the grid dimensions have changed (changed in the sense that
    // the grid has grown outwards)
    auto min_gd =
        *std::min_element(grid_dimensions_.begin(), grid_dimensions_.end());
    auto max_gd =
        *std::max_element(grid_dimensions_.begin(), grid_dimensions_.end());
    if (min_gd < threshold_dimensions_[0]) {
      threshold_dimensions_[0] = min_gd;
      has_grown_ = true;
    }
    if (max_gd > threshold_dimensions_[1]) {
      Log::Info("Grid",
                "Your simulation objects are getting near the edge of "
                "the simulation space. Be aware of boundary conditions that "
                "may come into play!");
      threshold_dimensions_[1] = max_gd;
      has_grown_ = true;
    }
  }

  /// Calculates what the grid dimensions need to be in order to contain all the
  /// simulation objects
  void CalculateGridDimensions(std::array<double, 6>* ret_grid_dimensions) {
    auto* rm = Simulation::GetActive()->GetResourceManager();

    const auto max_threads = omp_get_max_threads();
    // allocate version for each thread - avoid false sharing by padding them
    // assumes 64 byte cache lines (8 * sizeof(double))
    std::vector<std::array<double, 8>> xmin(max_threads, {{Math::kInfinity}});
    std::vector<std::array<double, 8>> ymin(max_threads, {{Math::kInfinity}});
    std::vector<std::array<double, 8>> zmin(max_threads, {{Math::kInfinity}});

    std::vector<std::array<double, 8>> xmax(max_threads, {{-Math::kInfinity}});
    std::vector<std::array<double, 8>> ymax(max_threads, {{-Math::kInfinity}});
    std::vector<std::array<double, 8>> zmax(max_threads, {{-Math::kInfinity}});

    std::vector<std::array<double, 8>> largest(max_threads, {{0}});

    rm->ApplyOnAllElementsParallelDynamic(1000, [&](SimObject* so, SoHandle) {
      auto tid = omp_get_thread_num();
      const auto& position = so->GetPosition();
      // x
      if (position[0] < xmin[tid][0]) {
        xmin[tid][0] = position[0];
      }
      if (position[0] > xmax[tid][0]) {
        xmax[tid][0] = position[0];
      }
      // y
      if (position[1] < ymin[tid][0]) {
        ymin[tid][0] = position[1];
      }
      if (position[1] > ymax[tid][0]) {
        ymax[tid][0] = position[1];
      }
      // z
      if (position[2] < zmin[tid][0]) {
        zmin[tid][0] = position[2];
      }
      if (position[2] > zmax[tid][0]) {
        zmax[tid][0] = position[2];
      }
      // larget object
      auto diameter = so->GetDiameter();
      if (diameter > largest[tid][0]) {
        largest[tid][0] = diameter;
      }
    });

    // reduce partial results into global one
    double& gxmin = (*ret_grid_dimensions)[0];
    double& gxmax = (*ret_grid_dimensions)[1];
    double& gymin = (*ret_grid_dimensions)[2];
    double& gymax = (*ret_grid_dimensions)[3];
    double& gzmin = (*ret_grid_dimensions)[4];
    double& gzmax = (*ret_grid_dimensions)[5];
    for (int tid = 0; tid < max_threads; tid++) {
      // x
      if (xmin[tid][0] < gxmin) {
        gxmin = xmin[tid][0];
      }
      if (xmax[tid][0] > gxmax) {
        gxmax = xmax[tid][0];
      }
      // y
      if (ymin[tid][0] < gymin) {
        gymin = ymin[tid][0];
      }
      if (ymax[tid][0] > gymax) {
        gymax = ymax[tid][0];
      }
      // z
      if (zmin[tid][0] < gzmin) {
        gzmin = zmin[tid][0];
      }
      if (zmax[tid][0] > gzmax) {
        gzmax = zmax[tid][0];
      }
      // larget object
      if (largest[tid][0] > largest_object_size_) {
        largest_object_size_ = largest[tid][0];
      }
    }
  }

  void RoundOffGridDimensions(const std::array<double, 6>& grid_dimensions) {
    assert(grid_dimensions_[0] > -9.999999999);
    assert(grid_dimensions_[2] > -9.999999999);
    assert(grid_dimensions_[4] > -9.999999999);
    assert(grid_dimensions_[1] < 80);
    assert(grid_dimensions_[3] < 80);
    assert(grid_dimensions_[5] < 80);
    grid_dimensions_[0] = floor(grid_dimensions[0]);
    grid_dimensions_[2] = floor(grid_dimensions[2]);
    grid_dimensions_[4] = floor(grid_dimensions[4]);
    grid_dimensions_[1] = ceil(grid_dimensions[1]);
    grid_dimensions_[3] = ceil(grid_dimensions[3]);
    grid_dimensions_[5] = ceil(grid_dimensions[5]);
  }

  /// @brief      Gets the Moore (i.e adjacent) boxes of the query boxAlso adds
  /// the
  ///             query box.
  ///
  /// @param[out] neighbor_boxes  The neighbor boxes
  /// @param[in]  box_idx         The query box
  ///
  void GetMooreBoxes(FixedSizeVector<const Box*, 27>* neighbor_boxes,
                     size_t box_idx) const {
    neighbor_boxes->push_back(GetBoxPointer(box_idx));

    // Adjacent 6 (top, down, left, right, front and back)
    if (adjacency_ >= kLow) {
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - 1));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + 1));
    }

    // Adjacent 12
    if (adjacency_ >= kMedium) {
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx - num_boxes_xy_ + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0]));
      neighbor_boxes->push_back(GetBoxPointer(box_idx + num_boxes_xy_ + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_axis_[0] + 1));
    }

    // Adjacent 8
    if (adjacency_ >= kHigh) {
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ - num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx - num_boxes_xy_ + num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1));
      neighbor_boxes->push_back(
          GetBoxPointer(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1));
    }
  }

  /// @brief      Gets the box indices of all adjacent boxes. Also adds the
  ///             query box index.
  ///
  /// @param[out] box_indices     Result containing all box indices
  /// @param[in]  box_idx         The query box
  ///
  void GetMooreBoxIndices(FixedSizeVector<uint64_t, 27>* box_indices,
                          size_t box_idx) const {
    box_indices->push_back(box_idx);

    // Adjacent 6 (top, down, left, right, front and back)
    if (adjacency_ >= kLow) {
      box_indices->push_back(box_idx - num_boxes_xy_);
      box_indices->push_back(box_idx + num_boxes_xy_);
      box_indices->push_back(box_idx - num_boxes_axis_[0]);
      box_indices->push_back(box_idx + num_boxes_axis_[0]);
      box_indices->push_back(box_idx - 1);
      box_indices->push_back(box_idx + 1);
    }

    // Adjacent 12
    if (adjacency_ >= kMedium) {
      box_indices->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[0]);
      box_indices->push_back(box_idx - num_boxes_xy_ - 1);
      box_indices->push_back(box_idx - num_boxes_axis_[0] - 1);
      box_indices->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0]);
      box_indices->push_back(box_idx + num_boxes_xy_ - 1);
      box_indices->push_back(box_idx + num_boxes_axis_[0] - 1);
      box_indices->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[0]);
      box_indices->push_back(box_idx - num_boxes_xy_ + 1);
      box_indices->push_back(box_idx - num_boxes_axis_[0] + 1);
      box_indices->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0]);
      box_indices->push_back(box_idx + num_boxes_xy_ + 1);
      box_indices->push_back(box_idx + num_boxes_axis_[0] + 1);
    }

    // Adjacent 8
    if (adjacency_ >= kHigh) {
      box_indices->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[0] - 1);
      box_indices->push_back(box_idx - num_boxes_xy_ - num_boxes_axis_[0] + 1);
      box_indices->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[0] - 1);
      box_indices->push_back(box_idx - num_boxes_xy_ + num_boxes_axis_[0] + 1);
      box_indices->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1);
      box_indices->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1);
      box_indices->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1);
      box_indices->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1);
    }
  }

  /// Determines current box based on parameter box_idx and adds it together
  /// with half of the surrounding boxes to the vector.
  /// Legend: C = center, N = north, E = east, S = south, W = west, F = front,
  ///         B = back
  /// For each box pair which is centro-symmetric only one box is taken --
  /// e.g. E-W: E, or BNW-FSE: BNW
  /// NB: for the update mechanism using a CircularBuffer the order is
  /// important.
  ///
  ///        (x-axis to the right \ y-axis up)
  ///        z=1
  ///        +-----+----+-----+
  ///        | BNW | BN | BNE |
  ///        +-----+----+-----+
  ///        | NW  | N  | NE  |
  ///        +-----+----+-----+
  ///        | FNW | FN | FNE |
  ///        +-----+----+-----+
  ///
  ///        z = 0
  ///        +-----+----+-----+
  ///        | BW  | B  | BE  |
  ///        +-----+----+-----+
  ///        | W   | C  | E   |
  ///        +-----+----+-----+
  ///        | FW  | F  | FE  |
  ///        +-----+----+-----+
  ///
  ///        z = -1
  ///        +-----+----+-----+
  ///        | BSW | BS | BSE |
  ///        +-----+----+-----+
  ///        | SW  | S  | SE  |
  ///        +-----+----+-----+
  ///        | FSW | FS | FSE |
  ///        +-----+----+-----+
  ///
  void GetHalfMooreBoxIndices(FixedSizeVector<size_t, 14>* neighbor_boxes,
                              size_t box_idx) const {
    // C
    neighbor_boxes->push_back(box_idx);
    // BW
    neighbor_boxes->push_back(box_idx + num_boxes_axis_[0] - 1);
    // FNW
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] - 1);
    // NW
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ - 1);
    // BNW
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] - 1);
    // B
    neighbor_boxes->push_back(box_idx + num_boxes_axis_[0]);
    // FN
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0]);
    // N
    neighbor_boxes->push_back(box_idx + num_boxes_xy_);
    // BN
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0]);
    // E
    neighbor_boxes->push_back(box_idx + 1);
    // BE
    neighbor_boxes->push_back(box_idx + num_boxes_axis_[0] + 1);
    // FNE
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ - num_boxes_axis_[0] + 1);
    // NE
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ + 1);
    // BNE
    neighbor_boxes->push_back(box_idx + num_boxes_xy_ + num_boxes_axis_[0] + 1);
  }

  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  const Box* GetBoxPointer(size_t index) const { return &(boxes_[index]); }

  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  Box* GetBoxPointer(size_t index) { return &(boxes_[index]); }

  /// Returns the box index in the one dimensional array based on box
  /// coordinates in space
  ///
  /// @param      box_coord  box coordinates in space (x, y, z)
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const std::array<uint32_t, 3>& box_coord) const {
    return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] +
           box_coord[0];
  }
};

}  // namespace bdm

#endif  // CORE_GRID_H_
