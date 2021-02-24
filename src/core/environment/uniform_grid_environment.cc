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

namespace bdm {

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::LoadBalanceInfoUG(
    UniformGridEnvironment* grid)
    : grid_(grid) {}

// -----------------------------------------------------------------------------
UniformGridEnvironment::LoadBalanceInfoUG::~LoadBalanceInfoUG() {}

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::Update() {
  mo_.Update(grid_->num_boxes_axis_);

  AllocateMemory();
  InitializeVectors();
  CalcPrefixSum();
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
  InitializeVectorFunctor f(grid_, 0, sorted_boxes_, cummulated_agents_);
  mo_.CallMortonIteratorConsumer(0, grid_->total_num_boxes_, f);
}

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::CalcPrefixSum() {
  for (uint64_t i = 1; i < grid_->total_num_boxes_; ++i) {
    cummulated_agents_[i] += cummulated_agents_[i - 1];
  }
}

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::CallAHIteratorConsumer(
    uint64_t start, Functor<void, Iterator<AgentHandle>>& f) const {}

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
    std::array<uint64_t, 3> box_coord;
    libmorton::morton3D_64_decode(morton_code, box_coord[0], box_coord[1],
                                  box_coord[2]);
    auto* box = grid->GetBoxPointer(grid->GetBoxIndex(box_coord));
    sorted_boxes[start] = box;
    cummulated_agents[start] = box->Size(grid->timestamp_);
    start++;
  }
}

// FIXME remove
void UniformGridEnvironment::LoadBalanceInfoUG::Iterate(
    Functor<void, const AgentHandle&>& callback) {
  for (uint64_t i = 0; i < grid_->total_num_boxes_; i++) {
    auto it = sorted_boxes_[i]->begin();
    while (!it.IsAtEnd()) {
      callback(*it);
      ++it;
    }
  }
}

}  // namespace bdm
