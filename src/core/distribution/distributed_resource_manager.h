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

#ifdef USE_DSE

#include "core/resource_manager.h"

namespace bdm {
namespace experimental {

class DistributedResourceManager : public ResourceManager {
 public:
  using AuraAgents = std::unordered_map<int, std::vector<Agent*>>;

  explicit DistributedResourceManager(TRootIOCtor* r);
  DistributedResourceManager();
  virtual ~DistributedResourceManager();

  Agent* GetAgent(AgentHandle ah) override;
  int GetNumAgentsInAura(int neighbor_rank) const;

  void ForEachAgentInAuraParallel(Functor<void, Agent*, AgentHandle>& functor);

  void UpdateAura(const AuraAgents& aura_agents);

 private:
  AuraAgents aura_agents_;

  BDM_CLASS_DEF_NV(DistributedResourceManager, 1);
};

}  // namespace experimental
}  // namespace bdm

#endif  // USE_DSE

#endif  // CORE_RANDOMIZED_RM_H_
