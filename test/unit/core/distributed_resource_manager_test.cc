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

#include "core/distribution/distributed_resource_manager.h"
#include <gtest/gtest.h>
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
TEST(DistributedResourceManager, All) {
  Simulation simulation(TEST_NAME);

  // setup
  DistributedResourceManager drm;
  drm.AddAgent(new TestAgent(12));
  //   create aura agents
  DistributedResourceManager::AuraAgents aura_agents;
  aura_agents[2].push_back(new TestAgent(20));
  aura_agents[2].push_back(new TestAgent(21));
  aura_agents[5].push_back(new TestAgent(50));
  //
  drm.UpdateAura(aura_agents);

  // verify
  //   GetNumAgentsInAura
  EXPECT_EQ(0, drm.GetNumAgentsInAura(0));
  EXPECT_EQ(2, drm.GetNumAgentsInAura(2));
  EXPECT_EQ(1, drm.GetNumAgentsInAura(5));
  //   GetAgent
  EXPECT_EQ(
      12, static_cast<TestAgent*>(drm.GetAgent(AgentHandle(0, 0)))->GetData());
  EXPECT_EQ(20, static_cast<TestAgent*>(drm.GetAgent(AgentHandle(true, 2, 0)))
                    ->GetData());
  EXPECT_EQ(21, static_cast<TestAgent*>(drm.GetAgent(AgentHandle(true, 2, 1)))
                    ->GetData());
  EXPECT_EQ(50, static_cast<TestAgent*>(drm.GetAgent(AgentHandle(true, 5, 0)))
                    ->GetData());

  //   ForEachAgentInAuraParallel
  std::set<int> agent_data;
  auto functor = L2F([&](Agent* agent, AgentHandle ah) {
#pragma omp critical
    agent_data.insert(bdm_static_cast<TestAgent*>(agent)->GetData());
  });
  std::set<int> expected = {20, 21, 50};
  drm.ForEachAgentInAuraParallel(functor);
  EXPECT_EQ(expected, agent_data);
}

}  // namespace experimental
}  // namespace bdm
