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

#ifndef CORE_RANDOMIZED_RM_H_
#define CORE_RANDOMIZED_RM_H_

#include <algorithm>
#include "core/resource_manager.h"
#ifdef LINUX
#include <parallel/algorithm>
#endif  // LINUX

namespace bdm {

/// A specialized implementation of ResourceManager that randomly shuffles the
/// order of agents to enable looping over agents randomly. By default, the
/// random reordering happens at the end of every iterations (during the
/// teardown operation), but this can be turned off by initializing the
/// `auto_randomize_` variable with `false`. To manually trigger the random
/// reordering of agents, you can call the `RandomizeAgentsOrder` function.
template <typename TBaseRm>
class RandomizedRm : public TBaseRm {
 public:
  explicit RandomizedRm(TRootIOCtor* r) {}
  RandomizedRm(bool auto_randomize = true);
  virtual ~RandomizedRm();

  void RandomizeAgentsOrder();
  void EndOfIteration() override;

 protected:
  // Automatically randomize the agent order at the end of each iteration
  // If false, you can call RandomizeAgentsOrder() manually
  bool auto_randomize_ = true;
  BDM_CLASS_DEF_NV(RandomizedRm, 1);
};

// -----------------------------------------------------------------------------
template <typename TBaseRm>
RandomizedRm<TBaseRm>::RandomizedRm(bool auto_randomize)
    : auto_randomize_(auto_randomize){};

// -----------------------------------------------------------------------------
template <typename TBaseRm>
RandomizedRm<TBaseRm>::~RandomizedRm() = default;

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

template <typename TBaseRm>
void RandomizedRm<TBaseRm>::RandomizeAgentsOrder() {
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

// -----------------------------------------------------------------------------
template <typename TBaseRm>
void RandomizedRm<TBaseRm>::EndOfIteration() {
  TBaseRm::EndOfIteration();
  if (auto_randomize_) {
    RandomizeAgentsOrder();
  }
}

}  // namespace bdm

#endif  // CORE_RANDOMIZED_RM_H_
