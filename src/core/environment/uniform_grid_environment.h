// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

  virtual ~UniformGridEnvironment() = default;

  /// Clears the grid
  void Clear() override {
    if (!is_custom_box_length_) {
      box_length_ = 1;
    }
    box_length_squared_ = 1;
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
      assert(idx <= std::numeric_limits<uint32_t>::max());
      agent->SetBoxIdx(static_cast<uint32_t>(idx));
    }

   private:
    UniformGridEnvironment* grid_ = nullptr;
  };

  void SetBoxLength(int32_t bl) {
    box_length_ = bl;
    is_custom_box_length_ = true;
  }

  void SetDetermineSimSize(bool value) { determine_sim_size_ = value; }

  int32_t GetBoxLength() { return box_length_; }

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

  LoadBalanceInfo* GetLoadBalanceInfo() override {
    lbi_.Update();
    return &lbi_;
  }

  /// @brief      Return the box index in the one dimensional array of the box
  ///             that contains the position
  ///
  /// @param[in]  position  The position of the object
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const Double3& position) const {
    // Check if converstion can be done without loosing information
    assert(floor(position[0]) <= std::numeric_limits<int32_t>::max());
    assert(floor(position[1]) <= std::numeric_limits<int32_t>::max());
    assert(floor(position[2]) <= std::numeric_limits<int32_t>::max());
    std::array<uint64_t, 3> box_coord;
    box_coord[0] =
        (static_cast<int32_t>(floor(position[0])) - grid_dimensions_[0]) /
        box_length_;
    box_coord[1] =
        (static_cast<int32_t>(floor(position[1])) - grid_dimensions_[2]) /
        box_length_;
    box_coord[2] =
        (static_cast<int32_t>(floor(position[2])) - grid_dimensions_[4]) /
        box_length_;

    return GetBoxIndex(box_coord);
  }

  std::array<int32_t, 6> GetDimensions() const override {
    return grid_dimensions_;
  }

  /// Returns true if the provided point is inside the simulation domain.
  /// Compares the points coordinates against grid_dimensions_ (without bounding
  /// boxes).
  bool ContainedInGrid(const Double3& point) const {
    double xmin = static_cast<double>(grid_dimensions_[0]) + box_length_;
    double xmax = static_cast<double>(grid_dimensions_[1]) - box_length_;
    double ymin = static_cast<double>(grid_dimensions_[2]) + box_length_;
    double ymax = static_cast<double>(grid_dimensions_[3]) - box_length_;
    double zmin = static_cast<double>(grid_dimensions_[4]) + box_length_;
    double zmax = static_cast<double>(grid_dimensions_[5]) - box_length_;
    if (point[0] >= xmin && point[0] <= xmax && point[1] >= ymin &&
        point[1] <= ymax && point[2] >= zmin && point[2] <= zmax) {
      return true;
    } else {
      return false;
    }
  }

  std::array<int32_t, 2> GetDimensionThresholds() const override {
    return threshold_dimensions_;
  }

  void GetNumBoxesAxis(uint32_t* nba) {
    // Check if conversion can be done without loosing information
    assert(num_boxes_axis_[0] <= std::numeric_limits<uint32_t>::max());
    assert(num_boxes_axis_[1] <= std::numeric_limits<uint32_t>::max());
    assert(num_boxes_axis_[2] <= std::numeric_limits<uint32_t>::max());
    nba[0] = static_cast<uint32_t>(num_boxes_axis_[0]);
    nba[1] = static_cast<uint32_t>(num_boxes_axis_[1]);
    nba[2] = static_cast<uint32_t>(num_boxes_axis_[2]);
  }

  uint64_t GetNumBoxes() const { return boxes_.size(); }

  std::array<uint64_t, 3> GetBoxCoordinates(size_t box_idx) const {
    std::array<uint64_t, 3> box_coord;
    box_coord[2] = box_idx / num_boxes_xy_;
    auto remainder = box_idx % num_boxes_xy_;
    box_coord[1] = remainder / num_boxes_axis_[0];
    box_coord[0] = remainder % num_boxes_axis_[0];
    return box_coord;
  }

  /// @brief      Applies the given lambda to each neighbor of the specified
  ///             agent is within the squared radius.
  ///
  /// In simulation code do not use this function directly. Use the same
  /// function from the execution context (e.g. `InPlaceExecutionContext`)
  ///
  /// @param[in]  lambda    The operation as a lambda
  /// @param      query     The query object
  /// @param      squared_radius  The squared search radius (type: double*)
  ///
  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Agent& query, double squared_radius) override {
    ForEachNeighbor(lambda, query.GetPosition(), squared_radius, &query);
  }

  /// @brief      Applies the given lambda to each neighbor of the specified
  ///             position within the squared radius.
  ///
  /// In simulation code do not use this function directly. Use the same
  /// function from the execution context (e.g. `InPlaceExecutionContext`)
  ///
  /// @param[in]  lambda    The operation as a lambda
  /// @param      query_position  The query position
  /// @param      squared_radius  The squared search radius (type: double*)
  ///
  void ForEachNeighbor(Functor<void, Agent*, double>& lambda,
                       const Double3& query_position, double squared_radius,
                       const Agent* query_agent = nullptr) override {
    if (squared_radius > box_length_squared_) {
      Log::Fatal(
          "UniformGridEnvironment::ForEachNeighbor",
          "The requested search radius (", std::sqrt(squared_radius), ")",
          " of the neighborhood search exceeds the "
          "box length (",
          box_length_, "). The resulting neighborhood would be incomplete.");
    }
    const auto& position = query_position;
    // Use uint32_t for compatibility with Agent::GetBoxIdx();
    uint32_t idx{std::numeric_limits<uint32_t>::max()};
    if (query_agent != nullptr) {
      idx = query_agent->GetBoxIdx();
    }
    // If the point is not inside the inner grid (excluding the bounding boxes)
    // as well as there was no previous box index assigned to the agent, we
    // cannot reliably detect the neighbors and warn the user.
    if (!ContainedInGrid(query_position) &&
        idx == std::numeric_limits<uint32_t>::max()) {
      Log::Warning(
          "UniformGridEnvironment::ForEachNeighbor",
          "You provided a query_position that is outside of the environment. ",
          "Neighbor search is not supported in this case. \n",
          "query_position: ", query_position,
          "\ngrid_dimensions: ", grid_dimensions_[0] + box_length_, ", ",
          grid_dimensions_[1] - box_length_, ", ",
          grid_dimensions_[2] + box_length_, ", ",
          grid_dimensions_[3] - box_length_, ", ",
          grid_dimensions_[4] + box_length_, ", ",
          grid_dimensions_[5] - box_length_);
      return;
    }
    // Freshly created agents are initialized with the largest uint32_t number
    // available. The above line assumes that the agent has already been located
    // in the grid, but this assumption does not hold for new agents. Hence, for
    // new agents, we manually compute the box index. This is also necessary if
    // we want to find the neighbors of a arbitrary 3D coordinate rather than
    // the neighbors of an agent.
    if (idx == std::numeric_limits<uint32_t>::max()) {
      size_t idx_tmp = GetBoxIndex(position);
      // Check if conversion can be done without loosing information
      assert(idx_tmp <= std::numeric_limits<uint32_t>::max());
      idx = static_cast<uint32_t>(idx_tmp);
    }

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
        if (squared_distance[i] < squared_radius) {
          lambda(agents[i], squared_distance[i]);
        }
      }
      size = 0;
    };

    while (!ni.IsAtEnd()) {
      auto ah = *ni;
      // increment iterator already here to hide memory latency
      ++ni;
      auto* agent = rm->GetAgent(ah);
      if (agent != query_agent) {
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
  };

  /// @brief      Applies the given functor to each neighbor of the specified
  ///             agent that is within the same box as the query agent
  ///             or in the 26 surrounding boxes.
  ///
  /// In simulation code do not use this function directly. Use the same
  /// function from the execution context (e.g. `InPlaceExecutionContext`)
  ///
  /// @param[in]  functor    The operation as a functor
  /// @param[in]      query      The query object
  /// @param[in]      criteria   This parameter is ignored. Pass a nullptr.
  ///
  void ForEachNeighbor(Functor<void, Agent*>& functor, const Agent& query,
                       void* criteria) override;

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

      ~GridNeighborMutex() override = default;

      void lock() override {  // NOLINT
        for (auto idx : mutex_indices_) {
          auto& mutex = mutex_builder_->mutexes_[idx].mutex_;
          while (mutex.test_and_set(std::memory_order_acquire)) {
            // acquire lock and spin if another thread is holding it
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
      MutexWrapper() = default;
      MutexWrapper(const MutexWrapper&) {}
      std::atomic_flag mutex_ = ATOMIC_FLAG_INIT;
    };

    ~GridNeighborMutexBuilder() override = default;

    void Update() {
      auto* grid = static_cast<UniformGridEnvironment*>(
          Simulation::GetActive()->GetEnvironment());
      mutexes_.resize(grid->GetNumBoxes());
    }

    NeighborMutex* GetMutex(uint64_t box_idx) override;

   private:
    /// one mutex for each box in `UniformGridEnvironment::boxes_`
    std::vector<MutexWrapper> mutexes_;
  };

  /// Returns the `NeighborMutexBuilder`. The client use it to create a
  /// `NeighborMutex`.
  NeighborMutexBuilder* GetNeighborMutexBuilder() override {
    return nb_mutex_builder_.get();
  }

 protected:
  /// Updates the grid, as agents may have moved, added or deleted
  void UpdateImplementation() override;

 private:
  class LoadBalanceInfoUG : public LoadBalanceInfo {
   public:
    LoadBalanceInfoUG(UniformGridEnvironment* grid);
    virtual ~LoadBalanceInfoUG();
    void Update();
    void CallHandleIteratorConsumer(
        uint64_t start, uint64_t end,
        Functor<void, Iterator<AgentHandle>*>& f) const override;

   private:
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
  };

  /// The vector containing all the boxes in the grid
  /// Using parallel resize vector to enable parallel initialization and thus
  /// better scalability.
  ParallelResizeVector<Box> boxes_;
  /// is incremented at each call to Update
  /// This is used to decide if boxes should be reinitialized
  uint32_t timestamp_ = 0;
  /// Length of a Box
  int32_t box_length_ = 1;
  /// Length of a Box squared
  int32_t box_length_squared_ = 1;
  /// True when the box length was set manually
  bool is_custom_box_length_ = false;
  /// If set to true, the UniformGridEnvironment determines the size of the
  /// simulation space automatically.
  /// If false, it uses param->min_bound and param->max_bound for each dimension
  bool determine_sim_size_ = true;
  /// Stores the number of Boxes for each axis
  std::array<uint64_t, 3> num_boxes_axis_ = {{0}};
  /// Number of boxes in the xy plane (=num_boxes_axis_[0] * num_boxes_axis_[1])
  size_t num_boxes_xy_ = 0;
  /// The total number of boxes in the uniform grid
  uint64_t total_num_boxes_ = 0;
  /// Implements linked list - array index = key, value: next element
  ///
  ///     // Usage
  ///     AgentHandle current_element = ...;
  ///     AgentHandle next_element = successors_[current_element];
  AgentVector<AgentHandle> successors_;
  /// Determines which boxes to search neighbors in (see enum Adjacency)
  Adjacency adjacency_;
  /// Cube which contains all agents
  /// {x_min, x_max, y_min, y_max, z_min, z_max}
  std::array<int32_t, 6> grid_dimensions_;
  /// Stores the min / max dimension value that need to be surpassed in order
  /// to trigger a diffusion grid change
  std::array<int32_t, 2> threshold_dimensions_;

  LoadBalanceInfoUG lbi_;  //!

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
    // Check if conversion can be done without loosing information
    assert(floor(grid_dimensions_[0]) >= std::numeric_limits<int32_t>::min());
    assert(floor(grid_dimensions_[2]) >= std::numeric_limits<int32_t>::min());
    assert(floor(grid_dimensions_[4]) >= std::numeric_limits<int32_t>::min());
    assert(ceil(grid_dimensions_[1]) <= std::numeric_limits<int32_t>::max());
    assert(ceil(grid_dimensions_[3]) <= std::numeric_limits<int32_t>::max());
    assert(ceil(grid_dimensions_[3]) <= std::numeric_limits<int32_t>::max());
    grid_dimensions_[0] = static_cast<int32_t>(floor(grid_dimensions[0]));
    grid_dimensions_[2] = static_cast<int32_t>(floor(grid_dimensions[2]));
    grid_dimensions_[4] = static_cast<int32_t>(floor(grid_dimensions[4]));
    grid_dimensions_[1] = static_cast<int32_t>(ceil(grid_dimensions[1]));
    grid_dimensions_[3] = static_cast<int32_t>(ceil(grid_dimensions[3]));
    grid_dimensions_[5] = static_cast<int32_t>(ceil(grid_dimensions[5]));
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
  const Box* GetBoxPointer(size_t index) const {
    assert(index < boxes_.size());
    return &(boxes_[index]);
  }

  /// @brief      Gets the pointer to the box with the given index
  ///
  /// @param[in]  index  The index of the box
  ///
  /// @return     The pointer to the box
  ///
  Box* GetBoxPointer(size_t index) {
    assert(index < boxes_.size());
    return &(boxes_[index]);
  }

  /// Returns the box index in the one dimensional array based on box
  /// coordinates in space
  ///
  /// @param      box_coord  box coordinates in space (x, y, z)
  ///
  /// @return     The box index.
  ///
  size_t GetBoxIndex(const std::array<uint64_t, 3>& box_coord) const {
    size_t box_idx = box_coord[2] * num_boxes_xy_ +
                     box_coord[1] * num_boxes_axis_[0] + box_coord[0];
    assert(box_idx < boxes_.size());
    return box_idx;
  }
};

}  // namespace bdm

#endif  // CORE_ENVIRONMENT_UNIFORM_GRID_ENVIRONMENT_H_
