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

#ifdef USE_DSE

#include "core/distribution/distributed_resource_manager.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
DistributedResourceManager::DistributedResourceManager(TRootIOCtor* r) {}

// -----------------------------------------------------------------------------
DistributedResourceManager::DistributedResourceManager() {}

// -----------------------------------------------------------------------------
DistributedResourceManager::~DistributedResourceManager() {}

// -----------------------------------------------------------------------------
Agent* DistributedResourceManager::GetAgent(AgentHandle ah) {
  if (!ah.IsInAura()) {
    return ResourceManager::GetAgent(ah);
  }
  assert(aura_agents_.find(ah.GetPrimaryIndex()) == aura_agents_.end());
  assert(ah.GetSecondaryIndex() < aura_agents_[ah.GetPrimaryIndex()].size());
  return aura_agents_[ah.GetPrimaryIndex()][ah.GetSecondaryIndex()];
}

// -----------------------------------------------------------------------------
int DistributedResourceManager::GetNumAgentsInAura(int neighbor_rank) const {
  if (aura_agents_.find(neighbor_rank) == aura_agents_.end()) {
    return 0;
  }
  return aura_agents_.at(neighbor_rank).size();
}

// -----------------------------------------------------------------------------
void DistributedResourceManager::ForEachAgentInAuraParallel(
    Functor<void, Agent*, AgentHandle>& functor) {
  for (auto& el : aura_agents_) {
    auto& agents = el.second;
#pragma omp parallel for
    for (uint64_t i = 0; i < agents.size(); ++i) {
      functor(agents[i], AgentHandle(true, el.first, i));
    }
  }
}

// -----------------------------------------------------------------------------
void DistributedResourceManager::UpdateAura(const AuraAgents& aura_agents) {
  aura_agents_ = aura_agents;
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_DSE
