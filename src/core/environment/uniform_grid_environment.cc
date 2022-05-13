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

#include "core/environment/uniform_grid_environment.h"
#include <morton/morton.h>  // NOLINT
#include "core/algorithm.h"

namespace bdm {

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::LoadBalanceInfoUG(
    UniformGridEnvironment* grid)
    : grid_(grid) {}

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::~LoadBalanceInfoUG() {}

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

  AgentHandleIterator(UniformGridEnvironment* grid, uint64_t start, uint64_t end, uint64_t box_index,
                      uint64_t discard, decltype(sorted_boxes) sorted_boxes)
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
    ~InitializeVectorFunctor() {}

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
  auto* query_ptr = &query;
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
    if (agent != query_ptr) {
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
