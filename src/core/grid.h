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

#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "core/container/fixed_size_vector.h"
#include "core/container/parallel_resize_vector.h"
#include "core/container/sim_object_vector.h"

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
 public:
  /// A single unit cube of the grid
  struct Box {
    /// start value of the linked list of simulatio objects inside this box.
    /// Next element can be found at `successors_[start_]`
    std::atomic<SoHandle> start_;
    /// length of the linked list (i.e. number of simulation objects)
    /// uint64_t, because sizeof(Box) = 16, for uint16_t and uint64_t
    std::atomic<uint64_t> length_;

    Box();
    /// Copy Constructor required for boxes_.resize()
    /// Since box values will be overwritten afterwards it forwards to the
    /// default ctor
    Box(const Box& other);

    Box& operator=(const Box& other);

    bool IsEmpty() const;

    /// @brief      Adds a simulation object to this box
    ///
    /// @param[in]  so       The object's identifier
    /// @param      successors   The successors
    ///
    /// @tparam     TSuccessors  Type of successors
    ///
    void AddObject(SoHandle so, SimObjectVector<SoHandle>* successors);

    /// An iterator that iterates over the cells in this box
    struct Iterator {
      Iterator(Grid* grid, const Box* box);

      bool IsAtEnd();

      Iterator& operator++();

      SoHandle operator*() const;

      /// Pointer to the neighbor grid; for accessing the successor_ list
      Grid* grid_;
      /// The current simulation object to be considered
      SoHandle current_value_;
      /// The remain number of simulation objects to consider
      int countdown_ = 0;
    };

    Iterator begin() const;  // NOLINT
  };

  /// An iterator that iterates over the boxes in this grid
  struct NeighborIterator {
    explicit NeighborIterator(
        const FixedSizeVector<const Box*, 27>& neighbor_boxes);

    bool IsAtEnd() const;

    SoHandle operator*() const;

    /// Version where empty neighbor boxes are allowed
    NeighborIterator& operator++();

   private:
    /// The 27 neighbor boxes that will be searched for simulation objects
    const FixedSizeVector<const Box*, 27>& neighbor_boxes_;
    /// The box that shall be considered to iterate over for finding simulation
    /// objects
    typename Box::Iterator box_iterator_;
    /// The id of the box to be considered (i.e. value between 0 - 26)
    uint16_t box_idx_ = 0;
    /// Flag to indicate that all the neighbor boxes have been searched through
    bool is_end_ = false;

    /// Forwards the iterator to the next non empty box and returns itself
    /// If there are no non empty boxes is_end_ is set to true
    NeighborIterator& ForwardToNonEmptyBox();
  };

  /// Enum that determines the degree of adjacency in search neighbor boxes
  //  todo(ahmad): currently only kHigh is supported (hardcoded 26 several
  //  places)
  enum Adjacency {
    kLow,    /**< The closest 8  neighboring boxes */
    kMedium, /**< The closest 18  neighboring boxes */
    kHigh    /**< The closest 26  neighboring boxes */
  };

  Grid();

  Grid(Grid const&) = delete;

  virtual ~Grid();

  void operator=(Grid const&) = delete;

  /// @brief      Initialize the grid with the given simulation objects
  /// @param[in]  adjacency    The adjacency (see #Adjacency)
  void Initialize(Adjacency adjacency = kHigh);


  /// Clears the grid
  void ClearGrid();

  /// Updates the grid, as simulation objects may have moved, added or deleted
  void UpdateGrid();

  /// @brief      Calculates the squared euclidian distance between two points
  ///             in 3D
  ///
  /// @param[in]  pos1  Position of the first point
  /// @param[in]  pos2  Position of the second point
  ///
  /// @return     The distance between the two points
  ///
  inline double SquaredEuclideanDistance(
      const std::array<double, 3>& pos1,
      const std::array<double, 3>& pos2) const;

  inline bool WithinSquaredEuclideanDistance(
      double squared_radius, const std::array<double, 3>& pos1,
      const std::array<double, 3>& pos2) const;

  void UpdateBoxZOrder();

  /// This method iterates over all elements. Iteration is performed in
  /// Z-order of boxes. There is no particular order for elements inside a box.
  void IterateZOrder(const std::function<void(const SoHandle&)>& lambda);

  /// @brief      Applies the given lambda to each neighbor
  ///
  /// @param[in]  lambda  The operation as a lambda
  /// @param      query   The query object
  ///
  /// @tparam     Lambda  The type of the lambda operation
  /// @tparam     SO      The type of the simulation object
  ///
  void ForEachNeighbor(const std::function<void(const SimObject*)>& lambda, const SimObject& query) const;

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
  void ForEachNeighborWithinRadius(const std::function<void(const SimObject*)>& lambda, const SimObject& query,
                                   double squared_radius);

  /// @brief      Return the box index in the one dimensional array of the box
  ///             that contains the position
  ///
  /// @param[in]  position  The position of the object
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const std::array<double, 3>& position) const;

  /// Gets the size of the largest object in the grid
  double GetLargestObjectSize() const;

  const std::array<int32_t, 6>& GetDimensions() const;

  const std::array<int32_t, 2>& GetDimensionThresholds() const;

  uint64_t GetNumBoxes() const;

  uint32_t GetBoxLength();

  bool HasGrown();

  std::array<uint32_t, 3> GetBoxCoordinates(size_t box_idx) const;

  bool IsInitialized();

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
      NeighborMutex(uint64_t box_idx,
                    const FixedSizeVector<uint64_t, 27>& mutex_indices,
                    NeighborMutexBuilder* mutex_builder);

      void lock();  // NOLINT

      void unlock();  // NOLINT

     private:
      uint64_t box_idx_;
      FixedSizeVector<uint64_t, 27> mutex_indices_;
      NeighborMutexBuilder* mutex_builder_;
    };

    /// Used to store mutexes in a vector.
    /// Always creates a new mutex (even for the copy constructor)
    struct MutexWrapper {
      MutexWrapper();
      MutexWrapper(const MutexWrapper&);
      std::atomic_flag mutex_ = ATOMIC_FLAG_INIT;
    };

    void Update();

    NeighborMutex GetMutex(uint64_t box_idx);

   private:
    /// one mutex for each box in `Grid::boxes_`
    std::vector<MutexWrapper> mutexes_;
  };

  /// Disable neighbor mutexes management. `GetNeighborMutexBuilder()` will
  /// return a nullptr.
  void DisableNeighborMutexes();

  /// Returns the `NeighborMutexBuilder`. The client use it to create a
  /// `NeighborMutex`. If neighbor mutexes has been disabled by calling
  /// `DisableNeighborMutexes`, this function will return a nullptr.
  NeighborMutexBuilder* GetNeighborMutexBuilder();

 private:
  /// The vector containing all the boxes in the grid
  /// Using parallel resize vector to enable parallel initialization and thus
  /// better scalability.
  ParallelResizeVector<Box> boxes_;
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

  void CheckGridGrowth();

  /// Calculates what the grid dimensions need to be in order to contain all the
  /// simulation objects
  void CalculateGridDimensions(std::array<double, 6>* ret_grid_dimensions);

  void RoundOffGridDimensions(const std::array<double, 6>& grid_dimensions);

  /// @brief      Gets the Moore (i.e adjacent) boxes of the query boxAlso adds
  /// the
  ///             query box.
  ///
  /// @param[out] neighbor_boxes  The neighbor boxes
  /// @param[in]  box_idx         The query box
  ///
  void GetMooreBoxes(FixedSizeVector<const Box*, 27>* neighbor_boxes,
                     size_t box_idx) const;

  /// @brief      Gets the box indices of all adjacent boxes. Also adds the
  ///             query box index.
  ///
  /// @param[out] box_indices     Result containing all box indices
  /// @param[in]  box_idx         The query box
  ///
  void GetMooreBoxIndices(FixedSizeVector<uint64_t, 27>* box_indices,
                          size_t box_idx) const;

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
                              size_t box_idx) const;

  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  const Box* GetBoxPointer(size_t index) const;

  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  Box* GetBoxPointer(size_t index);

  /// Returns the box index in the one dimensional array based on box
  /// coordinates in space
  ///
  /// @param      box_coord  box coordinates in space (x, y, z)
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const std::array<uint32_t, 3>& box_coord) const;
};

}  // namespace bdm

#endif  // CORE_GRID_H_
