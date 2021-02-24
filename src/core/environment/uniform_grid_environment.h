// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_ENVIRONMENT_UNIFORM_GRID_ENVIRONMENT_H_
#define CORE_ENVIRONMENT_UNIFORM_GRID_ENVIRONMENT_H_

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

#include <morton/morton.h>  // NOLINT

#include "core/container/agent_vector.h"
#include "core/container/fixed_size_vector.h"
#include "core/container/inline_vector.h"
#include "core/container/math_array.h"
#include "core/container/parallel_resize_vector.h"
#include "core/environment/environment.h"
#include "core/environment/morton_order.h"
#include "core/functor.h"
#include "core/load_balance_info.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/util/log.h"
#include "core/util/spinlock.h"

namespace bdm {

namespace detail {
struct InitializeGPUData;
}  // namespace detail

/// A class that represents Cartesian 3D grid
class UniformGridEnvironment : public Environment {
  // MechanicalForcesOpCuda needs access to some UniformGridEnvironment private
  // members to reconstruct
  // the grid on GPU (same for MechanicalForcesOpOpenCL)
  friend struct MechanicalForcesOpCuda;
  friend struct ::bdm::detail::InitializeGPUData;
  friend struct MechanicalForcesOpOpenCL;
  friend class SchedulerTest;

 public:
  /// A single unit cube of the grid
  struct Box {
    Spinlock lock_;
    // std::atomic<bool> timestamp_;
    uint32_t timestamp_;
    /// start value of the linked list of agents inside this box.
    /// Next element can be found at `successors_[start_]`
    AgentHandle start_;
    /// length of the linked list (i.e. number of agents)
    /// uint64_t, because sizeof(Box) = 16, for uint16_t and uint64_t
    uint16_t length_;

    Box() : timestamp_(0), start_(AgentHandle()), length_(0) {}
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

    uint16_t Size(uint64_t grid_timestamp) const {
      if (IsEmpty(grid_timestamp)) {
        return 0;
      }
      return length_;
    }

    /// @brief      Adds an agent to this box
    ///
    /// @param[in]  agent       The object's identifier
    /// @param   AddObject   successors   The successors
    void AddObject(AgentHandle ah, AgentVector<AgentHandle>* successors,
                   UniformGridEnvironment* grid) {
      std::lock_guard<Spinlock> lock_guard(lock_);

      if (timestamp_ != grid->timestamp_) {
        timestamp_ = grid->timestamp_;
        length_ = 1;
        start_ = ah;
      } else {
        length_++;
        (*successors)[ah] = start_;
        start_ = ah;
      }
    }

    /// An iterator that iterates over the cells in this box
    struct Iterator {
      Iterator(UniformGridEnvironment* grid, const Box* box)
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

      AgentHandle operator*() const { return current_value_; }

      /// Pointer to the neighbor grid; for accessing the successor_ list
      UniformGridEnvironment* grid_;
      /// The current agent to be considered
      AgentHandle current_value_;
      /// The remain number of agents to consider
      int countdown_ = 0;
    };

    Iterator begin() const {  // NOLINT
      auto* grid = static_cast<UniformGridEnvironment*>(
          Simulation::GetActive()->GetEnvironment());
      return Iterator(grid, this);
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

    AgentHandle operator*() const { return *box_iterator_; }

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
    /// The 27 neighbor boxes that will be searched for agents
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

  explicit UniformGridEnvironment(Adjacency adjacency = kHigh)
      : adjacency_(adjacency), lbi_(this) {}

  UniformGridEnvironment(UniformGridEnvironment const&) = delete;
  void operator=(UniformGridEnvironment const&) = delete;

  virtual ~UniformGridEnvironment() {}

  /// Clears the grid
  void Clear() override {
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

  struct AssignToBoxesFunctor : public Functor<void, Agent*, AgentHandle> {
    explicit AssignToBoxesFunctor(UniformGridEnvironment* grid) : grid_(grid) {}

    void operator()(Agent* agent, AgentHandle ah) override {
      const auto& position = agent->GetPosition();
      auto idx = grid_->GetBoxIndex(position);
      auto box = grid_->GetBoxPointer(idx);
      box->AddObject(ah, &(grid_->successors_), grid_);
      agent->SetBoxIdx(idx);
    }

   private:
    UniformGridEnvironment* grid_ = nullptr;
  };

  /// Updates the grid, as agents may have moved, added or deleted
  void Update() override {
    auto* rm = Simulation::GetActive()->GetResourceManager();

    if (rm->GetNumAgents() != 0) {
      Clear();
      timestamp_++;

      auto inf = Math::kInfinity;
      std::array<double, 6> tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
      CalcSimDimensionsAndLargestAgent(&tmp_dim, &largest_object_size_);
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
      total_num_boxes_ = num_boxes_xy_ * num_boxes_axis_[2];

      CheckGridGrowth();

      // resize boxes_
      if (boxes_.size() != total_num_boxes_) {
        if (boxes_.capacity() < total_num_boxes_) {
          boxes_.reserve(total_num_boxes_ * 2);
        }
        boxes_.resize(total_num_boxes_);
      }

      successors_.reserve();

      // Assign agents to boxes
      AssignToBoxesFunctor functor(this);
      rm->ForEachAgentParallel(1000, functor);
      auto* param = Simulation::GetActive()->GetParam();
      if (param->bound_space) {
        int min = param->min_bound;
        int max = param->max_bound;
        threshold_dimensions_ = {min, max};
      }

      if (param->thread_safety_mechanism ==
          Param::ThreadSafetyMechanism::kAutomatic) {
        nb_mutex_builder_->Update();
      }
    } else {
      // There are no agents in this simulation
      auto* param = Simulation::GetActive()->GetParam();

      bool uninitialized = boxes_.size() == 0;
      if (uninitialized && param->bound_space) {
        // Simulation has never had any agents
        // Initialize grid dimensions with `Param::min_bound` and
        // `Param::max_bound`
        // This is required for the DiffusionGrid
        int min = param->min_bound;
        int max = param->max_bound;
        grid_dimensions_ = {min, max, min, max, min, max};
        threshold_dimensions_ = {min, max};
        has_grown_ = true;
      } else if (!uninitialized) {
        // all agents have been removed in the last iteration
        // grid state remains the same, but we have to set has_grown_ to false
        // otherwise the DiffusionGrid will attempt to resize
        has_grown_ = false;
      } else {
        Log::Fatal(
            "UniformGridEnvironment",
            "You tried to initialize an empty simulation without bound space. "
            "Therefore we cannot determine the size of the simulation space. "
            "Please add agents, or set Param::bound_space, "
            "Param::min_bound, and Param::max_bound.");
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
    const uint32_t nx = num_boxes_axis_[0];
    const uint32_t ny = num_boxes_axis_[1];
    const uint32_t nz = num_boxes_axis_[2];
#pragma omp parallel for collapse(3)
    for (uint32_t x = 0; x < nx; x++) {
      for (uint32_t y = 0; y < ny; y++) {
        for (uint32_t z = 0; z < nz; z++) {
          auto box_idx = GetBoxIndex(std::array<uint64_t, 3>{x, y, z});
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
  void IterateZOrder(Functor<void, const AgentHandle&>& callback) override {
    lbi_.Update();
    lbi_.Iterate(callback);
    // UpdateBoxZOrder();
    // for (uint64_t i = 0; i < zorder_sorted_boxes_.size(); i++) {
    //   // if (lbi_.sorted_boxes_[i] != zorder_sorted_boxes_[i].second) {
    //   //   std::cout << "Difference at " << i << std::endl;
    //   // }
    //   auto it = zorder_sorted_boxes_[i].second->begin();
    //   while (!it.IsAtEnd()) {
    //     callback(*it);
    //     ++it;
    //   }
    // }
  }

  /// @brief      Applies the given lambda to each neighbor
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  void ForEachNeighbor(const std::function<void(const Agent*)>& lambda,
                       const Agent& query) const {
    auto idx = query.GetBoxIdx();

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    auto* rm = Simulation::GetActive()->GetResourceManager();

    NeighborIterator ni(neighbor_boxes, timestamp_);
    while (!ni.IsAtEnd()) {
      auto* agent = rm->GetAgent(*ni);
      if (agent != &query) {
        lambda(agent);
      }
      ++ni;
    }
  }

  /// @brief      Applies the given lambda to each neighbor or the specified
  ///             agent.
  ///
  /// In simulation code do not use this function directly. Use the same
  /// function from the exeuction context (e.g. `InPlaceExecutionContext`)
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  ///
  void ForEachNeighbor(Functor<void, const Agent*, double>& lambda,
                       const Agent& query) override {
    const auto& position = query.GetPosition();
    auto idx = query.GetBoxIdx();

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    auto* rm = Simulation::GetActive()->GetResourceManager();

    NeighborIterator ni(neighbor_boxes, timestamp_);
    const unsigned batch_size = 64;
    uint64_t size = 0;
    Agent* agents[batch_size] __attribute__((aligned(64)));
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
        lambda(agents[i], squared_distance[i]);
      }
      size = 0;
    };

    while (!ni.IsAtEnd()) {
      auto ah = *ni;
      // increment iterator already here to hide memory latency
      ++ni;
      auto* agent = rm->GetAgent(ah);
      if (agent != &query) {
        agents[size] = agent;
        const auto& pos = agent->GetPosition();
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
  ///             agent.
  ///
  /// In simulation code do not use this function directly. Use the same
  /// function from the exeuction context (e.g. `InPlaceExecutionContext`)
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  /// @param[in]  squared_radius  The search radius squared
  ///
  void ForEachNeighborWithinRadius(
      const std::function<void(const Agent*)>& lambda, const Agent& query,
      double squared_radius) {
    const auto& position = query.GetPosition();
    auto idx = query.GetBoxIdx();

    FixedSizeVector<const Box*, 27> neighbor_boxes;
    GetMooreBoxes(&neighbor_boxes, idx);

    auto* rm = Simulation::GetActive()->GetResourceManager();

    NeighborIterator ni(neighbor_boxes, timestamp_);
    while (!ni.IsAtEnd()) {
      // Do something with neighbor object
      auto* agent = rm->GetAgent(*ni);
      if (agent != &query) {
        const auto& neighbor_position = agent->GetPosition();
        if (this->WithinSquaredEuclideanDistance(squared_radius, position,
                                                 neighbor_position)) {
          lambda(agent);
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
    std::array<uint64_t, 3> box_coord;
    box_coord[0] = (floor(position[0]) - grid_dimensions_[0]) / box_length_;
    box_coord[1] = (floor(position[1]) - grid_dimensions_[2]) / box_length_;
    box_coord[2] = (floor(position[2]) - grid_dimensions_[4]) / box_length_;

    return GetBoxIndex(box_coord);
  }

  /// Gets the size of the largest object in the grid
  double GetLargestObjectSize() const override { return largest_object_size_; }

  const std::array<int32_t, 6>& GetDimensions() const override {
    return grid_dimensions_;
  }

  const std::array<int32_t, 2>& GetDimensionThresholds() const override {
    return threshold_dimensions_;
  }

  void GetNumBoxesAxis(uint32_t* nba) {
    nba[0] = num_boxes_axis_[0];
    nba[1] = num_boxes_axis_[1];
    nba[2] = num_boxes_axis_[2];
  }

  uint64_t GetNumBoxes() const { return boxes_.size(); }

  uint32_t GetBoxLength() { return box_length_; }

  std::array<uint64_t, 3> GetBoxCoordinates(size_t box_idx) const {
    std::array<uint64_t, 3> box_coord;
    box_coord[2] = box_idx / num_boxes_xy_;
    auto remainder = box_idx % num_boxes_xy_;
    box_coord[1] = remainder / num_boxes_axis_[0];
    box_coord[0] = remainder % num_boxes_axis_[0];
    return box_coord;
  }

  // NeighborMutex ---------------------------------------------------------

  /// This class ensures thread-safety for the InPlaceExecutionContext for the
  /// case
  /// that an agent modifies its neighbors.
  class GridNeighborMutexBuilder : public Environment::NeighborMutexBuilder {
   public:
    /// The NeighborMutex class is a synchronization primitive that can be
    /// used to protect agents data from being simultaneously accessed by
    /// multiple threads.
    class GridNeighborMutex
        : public Environment::NeighborMutexBuilder::NeighborMutex {
     public:
      GridNeighborMutex(const FixedSizeVector<uint64_t, 27>& mutex_indices,
                        GridNeighborMutexBuilder* mutex_builder)
          : mutex_indices_(mutex_indices), mutex_builder_(mutex_builder) {
        // Deadlocks occur if mutliple threads try to acquire the same locks,
        // but in different order.
        // -> sort to avoid deadlocks - see lock ordering
        std::sort(mutex_indices_.begin(), mutex_indices_.end());
      }

      virtual ~GridNeighborMutex() {}

      void lock() override {  // NOLINT
        for (auto idx : mutex_indices_) {
          auto& mutex = mutex_builder_->mutexes_[idx].mutex_;
          // acquire lock (and spin if another thread is holding it)
          while (mutex.test_and_set(std::memory_order_acquire)) {
          }
        }
      }

      void unlock() override {  // NOLINT
        for (auto idx : mutex_indices_) {
          auto& mutex = mutex_builder_->mutexes_[idx].mutex_;
          mutex.clear(std::memory_order_release);
        }
      }

      void SetMutexIndices(const FixedSizeVector<uint64_t, 27>& indices) {
        mutex_indices_ = indices;
        std::sort(mutex_indices_.begin(), mutex_indices_.end());
      }

     private:
      FixedSizeVector<uint64_t, 27> mutex_indices_;
      GridNeighborMutexBuilder* mutex_builder_;
    };

    /// Used to store mutexes in a vector.
    /// Always creates a new mutex (even for the copy constructor)
    struct MutexWrapper {
      MutexWrapper() {}
      MutexWrapper(const MutexWrapper&) {}
      std::atomic_flag mutex_ = ATOMIC_FLAG_INIT;
    };

    virtual ~GridNeighborMutexBuilder() {}

    void Update() {
      auto* grid = static_cast<UniformGridEnvironment*>(
          Simulation::GetActive()->GetEnvironment());
      mutexes_.resize(grid->GetNumBoxes());
    }

    NeighborMutex* GetMutex(uint64_t box_idx) override {
      auto* grid = static_cast<UniformGridEnvironment*>(
          Simulation::GetActive()->GetEnvironment());
      FixedSizeVector<uint64_t, 27> box_indices;
      grid->GetMooreBoxIndices(&box_indices, box_idx);
      thread_local GridNeighborMutex* mutex =
          new GridNeighborMutex(box_indices, this);
      mutex->SetMutexIndices(box_indices);
      return mutex;
    }

   private:
    /// one mutex for each box in `UniformGridEnvironment::boxes_`
    std::vector<MutexWrapper> mutexes_;
  };

  /// Returns the `NeighborMutexBuilder`. The client use it to create a
  /// `NeighborMutex`.
  NeighborMutexBuilder* GetNeighborMutexBuilder() override {
    return nb_mutex_builder_.get();
  }

 private:
  class LoadBalanceInfoUG : public LoadBalanceInfo {
   public:
    LoadBalanceInfoUG(UniformGridEnvironment* grid);
    virtual ~LoadBalanceInfoUG();
    void Update();
    void CallAHIteratorConsumer(
        uint64_t start, Functor<void, Iterator<AgentHandle>>& f) const override;
    // FIXME delete
    void Iterate(Functor<void, const AgentHandle&>& callback);
    // private:
    UniformGridEnvironment* grid_;
    MortonOrder mo_;
    ParallelResizeVector<Box*> sorted_boxes_;
    ParallelResizeVector<uint64_t> cummulated_agents_;

    struct InitializeVectorFunctor : public Functor<void, Iterator<uint64_t>*> {
      UniformGridEnvironment* grid;
      uint64_t start;
      ParallelResizeVector<Box*>& sorted_boxes;
      ParallelResizeVector<uint64_t>& cummulated_agents;

      InitializeVectorFunctor(UniformGridEnvironment* grid, uint64_t start,
                              decltype(sorted_boxes) sorted_boxes,
                              decltype(cummulated_agents) cummulated_agents);
      virtual ~InitializeVectorFunctor();

      void operator()(Iterator<uint64_t>* it) override;
    };

    void AllocateMemory();
    void InitializeVectors();
    void CalcPrefixSum();
  };

  /// The vector containing all the boxes in the grid
  /// Using parallel resize vector to enable parallel initialization and thus
  /// better scalability.
  ParallelResizeVector<Box> boxes_;
  /// is incremented at each call to Update
  /// This is used to decide if boxes should be reinitialized
  uint32_t timestamp_ = 0;
  /// Length of a Box
  uint32_t box_length_ = 1;
  /// Stores the number of boxes for each axis
  std::array<uint64_t, 3> num_boxes_axis_ = {{0}};
  /// Number of boxes in the xy plane (=num_boxes_axis_[0] * num_boxes_axis_[1])
  size_t num_boxes_xy_ = 0;
  uint64_t total_num_boxes_ = 0;
  /// Implements linked list - array index = key, value: next element
  ///
  ///     // Usage
  ///     AgentHandle current_element = ...;
  ///     AgentHandle next_element = successors_[current_element];
  AgentVector<AgentHandle> successors_;
  /// Determines which boxes to search neighbors in (see enum Adjacency)
  Adjacency adjacency_;
  /// The size of the largest object in the simulation
  double largest_object_size_ = 0;
  /// Cube which contains all agents
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  std::array<int32_t, 6> grid_dimensions_;
  /// Stores the min / max dimension value that need to be surpassed in order
  /// to trigger a diffusion grid change
  std::array<int32_t, 2> threshold_dimensions_;

  LoadBalanceInfoUG lbi_;  //!
  /// FIXME delete
  /// stores pairs of <box morton code,  box pointer> sorted by morton code.
  ParallelResizeVector<std::pair<uint32_t, const Box*>> zorder_sorted_boxes_;

  /// Holds instance of NeighborMutexBuilder.
  /// NeighborMutexBuilder is updated if `Param::thread_safety_mechanism`
  /// is set to `kAutomatic`
  std::unique_ptr<GridNeighborMutexBuilder> nb_mutex_builder_ =
      std::make_unique<GridNeighborMutexBuilder>();

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
      Log::Info("UniformGridEnvironment",
                "Your agents are getting near the edge of "
                "the simulation space. Be aware of boundary conditions that "
                "may come into play!");
      threshold_dimensions_[1] = max_gd;
      has_grown_ = true;
    }
  }

  void RoundOffGridDimensions(const std::array<double, 6>& grid_dimensions) {
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
  size_t GetBoxIndex(const std::array<uint64_t, 3>& box_coord) const {
    return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] +
           box_coord[0];
  }
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_UNIFORM_GRID_ENVIRONMENT_H_
