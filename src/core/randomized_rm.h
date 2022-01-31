// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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
#ifdef LINUX
#include <parallel/algorithm>
#endif  // LINUX

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

struct Ubrng {
  using result_type = uint32_t;
  Random* random;
  Ubrng(Random* random) : random(random) {}
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }
  result_type operator()() {
    return random->Integer(std::numeric_limits<result_type>::max());
  }
};

// -----------------------------------------------------------------------------
template <typename TBaseRm>
void RandomizedRm<TBaseRm>::EndOfIteration() {
  TBaseRm::EndOfIteration();
  // shuffle
#pragma omp parallel for schedule(static, 1)
  for (uint64_t n = 0; n < this->agents_.size(); ++n) {
#ifdef LINUX
    __gnu_parallel::random_shuffle(this->agents_[n].begin(),
                                   this->agents_[n].end());
#else
    auto* random = Simulation::GetActive()->GetRandom();
    std::shuffle(this->agents_[n].begin(), this->agents_[n].end(),
                 Ubrng(random));
#endif  // LINUX
  }

  // update uid_ah_map_
  auto update_agent_map = L2F([this](Agent* a, AgentHandle ah) {
    this->uid_ah_map_.Insert(a->GetUid(), ah);
  });
  TBaseRm::ForEachAgentParallel(update_agent_map);
}

}  // namespace bdm

#endif  // CORE_RANDOMIZED_RM_H_
