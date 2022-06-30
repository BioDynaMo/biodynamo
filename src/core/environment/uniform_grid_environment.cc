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

#include "core/environment/uniform_grid_environment.h"
#include <morton/morton.h>  // NOLINT
#include "core/algorithm.h"

namespace bdm {

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::LoadBalanceInfoUG(
    UniformGridEnvironment* grid)
    : grid_(grid) {}

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::~LoadBalanceInfoUG() = default;

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::Update() {
  if (grid_->total_num_boxes_ == 0) {
    return;
  }

  mo_.Update(grid_->num_boxes_axis_);

  AllocateMemory();
  InitializeVectors();
  InPlaceParallelPrefixSum(cummulated_agents_, grid_->total_num_boxes_);
}

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::AllocateMemory() {
  if (sorted_boxes_.capacity() < grid_->boxes_.size()) {
    sorted_boxes_.reserve(grid_->boxes_.capacity());
  }
  if (cummulated_agents_.capacity() < grid_->boxes_.size()) {
    cummulated_agents_.reserve(grid_->boxes_.capacity());
  }
}

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::InitializeVectors() {
#pragma omp parallel
  {
    auto* ti = ThreadInfo::GetInstance();
    auto num_threads = ti->GetMaxThreads();
    auto tid = ti->GetMyThreadId();
    // use static scheduling
    auto correction = grid_->total_num_boxes_ % num_threads == 0 ? 0 : 1;
    auto chunk = grid_->total_num_boxes_ / num_threads + correction;
    auto start = tid * chunk;
    auto end = std::min(grid_->total_num_boxes_, start + chunk);

    InitializeVectorFunctor f(grid_, start, sorted_boxes_, cummulated_agents_);
    mo_.CallMortonIteratorConsumer(start, end - 1, f);
  }
}

// -----------------------------------------------------------------------------
struct AgentHandleIterator : public Iterator<AgentHandle> {
  UniformGridEnvironment* grid_;
  uint64_t start, end, box_index, discard;
  const ParallelResizeVector<UniformGridEnvironment::Box*>& sorted_boxes;
  UniformGridEnvironment::Box::Iterator box_it;
  uint64_t tid;

  AgentHandleIterator(UniformGridEnvironment* grid, uint64_t start,
                      uint64_t end, uint64_t box_index, uint64_t discard,
                      decltype(sorted_boxes) sorted_boxes)
      : grid_(grid),
        start(start),
        end(end),
        box_index(box_index),
        discard(discard),
        sorted_boxes(sorted_boxes),
        box_it(sorted_boxes[box_index]->begin(grid)) {
    // discard elements
    tid = ThreadInfo::GetInstance()->GetMyThreadId();
    for (uint64_t i = 0; i < discard; ++i) {
      Next();
      this->start--;
    }
  }

  bool HasNext() const override { return start < end; }

  AgentHandle Next() override {
    while (box_it.IsAtEnd()) {
      box_index++;
      box_it = sorted_boxes[box_index]->begin(grid_);
    }
    auto ret = *box_it;
    start++;
    ++box_it;
    return ret;
  }
};

// -----------------------------------------------------------------------------

void UniformGridEnvironment::UpdateImplementation() {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  if (rm->GetNumAgents() != 0) {
    Clear();
    timestamp_++;

    auto* param = Simulation::GetActive()->GetParam();
    if (determine_sim_size_) {
      auto inf = Math::kInfinity;
      std::array<real_t, 6> tmp_dim = {{inf, -inf, inf, -inf, inf, -inf}};
      CalcSimDimensionsAndLargestAgent(&tmp_dim);
      RoundOffGridDimensions(tmp_dim);
    } else {
      grid_dimensions_[0] = static_cast<int>(floor(param->min_bound));
      grid_dimensions_[2] = static_cast<int>(floor(param->min_bound));
      grid_dimensions_[4] = static_cast<int>(floor(param->min_bound));
      grid_dimensions_[1] = static_cast<int>(ceil(param->max_bound));
      grid_dimensions_[3] = static_cast<int>(ceil(param->max_bound));
      grid_dimensions_[5] = static_cast<int>(ceil(param->max_bound));
    }

    // If the box_length_ is not set manually, we set it to the largest agent
    // size
    if (!is_custom_box_length_ && determine_sim_size_) {
      auto los = ceil(GetLargestAgentSize());
      assert(los > 0 &&
             "The largest object size was found to be 0. Please check if your "
             "cells are correctly initialized.");
      box_length_ = los;
    } else if (!is_custom_box_length_ && !determine_sim_size_) {
      Log::Fatal("UniformGridEnvironment",
                 "No box length specified although determine_sim_size_ is "
                 "set to false. Call the member function "
                 "SetBoxLength(box_length), or SetDetermineSimSize(false).");
    }
    box_length_squared_ = box_length_ * box_length_;

    if (!determine_sim_size_) {
      this->largest_object_size_ = box_length_;
      this->largest_object_size_squared_ = box_length_squared_;
    }

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
    rm->ForEachAgentParallel(param->scheduling_batch_size, functor);
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

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::CallHandleIteratorConsumer(
    uint64_t start, uint64_t end,
    Functor<void, Iterator<AgentHandle>*>& f) const {
  if (grid_->total_num_boxes_ == 0 || end <= start) {
    return;
  }
  auto index =
      BinarySearch(start, cummulated_agents_, 0, grid_->total_num_boxes_ - 1) +
      1;
  AgentHandleIterator it(grid_, start, end, index,
                         start - cummulated_agents_[index - 1], sorted_boxes_);
  f(&it);
}

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::InitializeVectorFunctor::
    InitializeVectorFunctor(UniformGridEnvironment* grid, uint64_t start,
                            decltype(sorted_boxes) sorted_boxes,
                            decltype(cummulated_agents) cummulated_agents)
    : grid(grid),
      start(start),
      sorted_boxes(sorted_boxes),
      cummulated_agents(cummulated_agents) {}

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::InitializeVectorFunctor::
    ~InitializeVectorFunctor() = default;

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::InitializeVectorFunctor::
operator()(Iterator<uint64_t>* it) {
  while (it->HasNext()) {
    auto morton_code = it->Next();
    uint_fast32_t x, y, z;
    libmorton::morton3D_64_decode(morton_code, x, y, z);
    auto* box = grid->GetBoxPointer(grid->GetBoxIndex(std::array<uint64_t, 3>{
        static_cast<uint64_t>(x), static_cast<uint64_t>(y),
        static_cast<uint64_t>(z)}));
    sorted_boxes[start] = box;
    cummulated_agents[start] = box->Size(grid->timestamp_);
    start++;
  }
}

// -----------------------------------------------------------------------------
using NeighborMutex = Environment::NeighborMutexBuilder::NeighborMutex;
using GridNeighborMutexBuilder =
    UniformGridEnvironment::GridNeighborMutexBuilder;

NeighborMutex* GridNeighborMutexBuilder::GetMutex(uint64_t box_idx) {
  auto* grid = static_cast<UniformGridEnvironment*>(
      Simulation::GetActive()->GetEnvironment());
  FixedSizeVector<uint64_t, 27> box_indices;
  grid->GetMooreBoxIndices(&box_indices, box_idx);
  thread_local GridNeighborMutex* mutex =
      new GridNeighborMutex(box_indices, this);
  mutex->SetMutexIndices(box_indices);
  return mutex;
}

// -----------------------------------------------------------------------------
void UniformGridEnvironment::ForEachNeighbor(Functor<void, Agent*>& functor,
                                             const Agent& query,
                                             void* criteria) {
  auto idx = query.GetBoxIdx();

  FixedSizeVector<const Box*, 27> neighbor_boxes;
  GetMooreBoxes(&neighbor_boxes, idx);

  auto* rm = Simulation::GetActive()->GetResourceManager();

  NeighborIterator ni(this, neighbor_boxes, timestamp_);
  const unsigned batch_size = 64;
  uint64_t size = 0;
  Agent* agents[batch_size] __attribute__((aligned(64)));

  auto process_batch = [&]() {
    for (uint64_t i = 0; i < size; ++i) {
      functor(agents[i]);
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
      size++;
      if (size == batch_size) {
        process_batch();
      }
    }
  }
  process_batch();
}

}  // namespace bdm
