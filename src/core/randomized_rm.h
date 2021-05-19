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

#ifndef CORE_RANDOMIZED_RM_H_
#define CORE_RANDOMIZED_RM_H_

#include <algorithm>
#include "core/resource_manager.h"

namespace bdm {

template <typename TBaseRm>
class RandomizedRm : public TBaseRm {
 public:
  explicit RandomizedRm(TRootIOCtor* r) {}
  RandomizedRm();
  virtual ~RandomizedRm();

  void EndOfIteration() override;

 protected:
  BDM_CLASS_DEF_NV(RandomizedRm, 1);
};

// -----------------------------------------------------------------------------
template <typename TBaseRm>
RandomizedRm<TBaseRm>::RandomizedRm() {}

// -----------------------------------------------------------------------------
template <typename TBaseRm>
RandomizedRm<TBaseRm>::~RandomizedRm() {}

// -----------------------------------------------------------------------------
template <typename TBaseRm>
void RandomizedRm<TBaseRm>::EndOfIteration() {
  TBaseRm::EndOfIteration();
  // shuffle
#pragma omp parallel for schedule(static, 1)
  for (uint64_t n = 0; n < this->agents_.size(); ++n) {
    // TODO(all) parallelize shuffling
    // see for example: https://arxiv.org/abs/1508.03167
    std::random_shuffle(this->agents_[n].begin(), this->agents_[n].end());
  }
  // update uid_ah_map_
  auto update_agent_map = L2F([this](Agent* a, AgentHandle ah) {
    this->uid_ah_map_.Insert(a->GetUid(), ah);
  });
  TBaseRm::ForEachAgentParallel(update_agent_map);
}

}  // namespace bdm

#endif  // CORE_RANDOMIZED_RM_H_
