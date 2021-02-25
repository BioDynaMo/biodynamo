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
  if (grid_->total_num_boxes_ == 0) {
    return;
  }

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
void UniformGridEnvironment::LoadBalanceInfoUG::CalcPrefixSum() {
  for (uint64_t i = 1; i < grid_->total_num_boxes_; ++i) {
    cummulated_agents_[i] += cummulated_agents_[i - 1];
    // std::cout << i << " - " << cummulated_agents_[i] << std::endl;
  }
}

// -----------------------------------------------------------------------------
struct AgentHandleIterator : public Iterator<AgentHandle> {
  uint64_t start, end, box_index, discard;
  const ParallelResizeVector<UniformGridEnvironment::Box*>& sorted_boxes;
  UniformGridEnvironment::Box::Iterator box_it;
  uint64_t tid;

  AgentHandleIterator(uint64_t start, uint64_t end, uint64_t box_index,
                      uint64_t discard, decltype(sorted_boxes) sorted_boxes)
      : start(start),
        end(end),
        box_index(box_index),
        discard(discard),
        sorted_boxes(sorted_boxes),
        box_it(sorted_boxes[box_index]->begin()) {
    // discard elements
    tid = ThreadInfo::GetInstance()->GetMyThreadId();
    for (uint64_t i = 0; i < discard; ++i) {
      Next();
      // #pragma omp critical
      //   std::cout << tid << " discard " << val << std::endl;
      this->start--;
    }
  }

  bool HasNext() const override { return start < end; }

  AgentHandle Next() override {
    // std::cout << "Next " << start << " " << end << std::endl;
    while (box_it.IsAtEnd()) {
      // #pragma omp critical
      //     std::cout << tid << "  box empty " << box_index << std::endl;
      box_index++;
      box_it = sorted_boxes[box_index]->begin();
    }
    auto ret = *box_it;
    // #pragma omp critical
    //     std::cout << tid << " Next " << ret << " box index " << box_index <<
    //     std::endl;
    start++;
    ++box_it;
    return ret;
  }
};

// -----------------------------------------------------------------------------
// if search_val is found in container, return right-most occurence.
// If not return the index of the right-most element that is smaller.
template <typename T>
uint64_t BinarySearch(uint64_t search_val, const T& container, uint64_t from,
                      uint64_t to) {
  // #pragma omp critical
  // std::cout << " sv " << search_val << " from " << from << " to " << to <<
  // std::endl;

  if (to <= from) {
    if (container[from] != search_val && from > 0) {
      from--;
    }
    // #pragma omp critical
    // std::cout << "  found2 " << from << std::endl;
    return from;
  }

  auto m = (from + to) / 2;
  // std::cout << " sv " << search_val << " from " << from << " to " << to << "
  // m "
  //           << m << " val[m] " << container[m].first << std::endl;
  if (container[m] == search_val) {
    if (m + 1 <= to && container[m + 1] == search_val) {
      return BinarySearch(search_val, container, m + 1, to);
    }
    // #pragma omp critical
    // std::cout << "  found " << m << std::endl;
    return m;
  } else if (container[m] > search_val) {
    return BinarySearch(search_val, container, from, m);
  } else {
    return BinarySearch(search_val, container, m + 1, to);
  }
}

// -----------------------------------------------------------------------------
void UniformGridEnvironment::LoadBalanceInfoUG::CallHandleIteratorConsumer(
    uint64_t start, uint64_t end,
    Functor<void, Iterator<AgentHandle>*>& f) const {
  if (grid_->total_num_boxes_ == 0 || end <= start) {
    return;
  }
  // #pragma omp critical
  //   std::cout << cummulated_agents_.size() << " - " <<
  //   cummulated_agents_.capacity() << std::endl;
  auto index =
      BinarySearch(start, cummulated_agents_, 0, grid_->total_num_boxes_ - 1) +
      1;
  // #pragma omp critical
  //   std::cout << ThreadInfo::GetInstance()->GetMyThreadId() << " start " <<
  //   start << " end " << end << " index " << index << " ca[index] " <<
  //   cummulated_agents_[index] << " ca[index  - 1] " <<
  // cummulated_agents_[index - 1] << std::endl;
  AgentHandleIterator it(start, end, index,
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
