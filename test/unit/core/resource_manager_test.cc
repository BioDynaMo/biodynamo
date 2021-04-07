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

// I/O related code must be in header file
#include "unit/core/resource_manager_test.h"
#include "core/model_initializer.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_agent.h"

namespace bdm {

TEST(ResourceManagerTest, ForEachAgent) { RunForEachAgentTest(); }

TEST(ResourceManagerTest, GetNumAgents) { RunGetNumAgents(); }

TEST(ResourceManagerTest, ForEachAgentParallel) {
  RunForEachAgentParallelTest();
}

#ifdef USE_DICT
TEST(ResourceManagerTest, IO) { RunIOTest(); }
#endif  // USE_DICT

TEST(ResourceManagerTest, PushBackAndGetAgentTest) {
  RunPushBackAndGetAgentTest();
}

TEST(ResourceManagerTest, RemoveAndContains) { RunRemoveAndContainsTest(); }

TEST(ResourceManagerTest, Clear) { RunClearTest(); }

TEST(ResourceManagerTest, SortAndForEachAgentParallel) {
  RunSortAndForEachAgentParallel();
}

TEST(ResourceManagerTest, SortAndForEachAgentParallelDynamic) {
  RunSortAndForEachAgentParallelDynamic();
}

TEST(ResourceManagerTest, DiffusionGrid) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  int counter = 0;
  auto count = [&](DiffusionGrid* dg) { counter++; };

  DiffusionGrid* dgrid_1 = new EulerGrid(0, "Kalium", 0.4, 0, 2);
  DiffusionGrid* dgrid_2 = new EulerGrid(1, "Natrium", 0.2, 0.1, 1);
  DiffusionGrid* dgrid_3 = new EulerGrid(2, "Calcium", 0.5, 0.1, 1);
  rm->AddDiffusionGrid(dgrid_1);
  rm->AddDiffusionGrid(dgrid_2);
  rm->AddDiffusionGrid(dgrid_3);

  rm->ForEachDiffusionGrid(count);
  ASSERT_EQ(3, counter);

  EXPECT_EQ(dgrid_1, rm->GetDiffusionGrid(0));
  EXPECT_EQ(dgrid_1, rm->GetDiffusionGrid("Kalium"));

  EXPECT_EQ(dgrid_2, rm->GetDiffusionGrid(1));
  EXPECT_EQ(dgrid_2, rm->GetDiffusionGrid("Natrium"));

  EXPECT_EQ(dgrid_3, rm->GetDiffusionGrid(2));
  EXPECT_EQ(dgrid_3, rm->GetDiffusionGrid("Calcium"));

  rm->RemoveDiffusionGrid(dgrid_2->GetSubstanceId());

  counter = 0;
  rm->ForEachDiffusionGrid(count);
  ASSERT_EQ(2, counter);
}

TEST(ResourceManagerTest, Defragmentation) {
  auto set_param = [](Param* param) {
    param->agent_uid_defragmentation_low_watermark = 0.3;
    param->agent_uid_defragmentation_high_watermark = 0.8;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* agent_uid_generator = simulation.GetAgentUidGenerator();

  // we don't know the how big the internal agent uid map is
  rm->AddAgent(new TestAgent());
  rm->EndOfIteration();
  EXPECT_TRUE(agent_uid_generator->IsInDefragmentationMode());
  // fill it to the max
  uint64_t cnt = 1;
  while (agent_uid_generator->IsInDefragmentationMode()) {
    rm->AddAgent(new TestAgent());
    cnt++;
  }
  // now we know how many agents are 100%
  rm->EndOfIteration();
  EXPECT_FALSE(agent_uid_generator->IsInDefragmentationMode());

  // remove enough agents to drop below the low watermark
  uint64_t remove = std::ceil(cnt * 0.7) + 1;
  while (remove != 0) {
    remove--;
    rm->RemoveAgent(AgentUid(remove));
  }
  rm->EndOfIteration();
  EXPECT_TRUE(agent_uid_generator->IsInDefragmentationMode());

  // add enough agents to exceed the high watermark
  uint64_t add = std::ceil(cnt * 0.5) + 1;
  while (add != 0) {
    add--;
    rm->AddAgent(new TestAgent());
  }
  rm->EndOfIteration();
  EXPECT_FALSE(agent_uid_generator->IsInDefragmentationMode());
  auto uid = agent_uid_generator->GenerateUid();
  EXPECT_GE(uid.GetIndex(), cnt);
  EXPECT_EQ(0u, uid.GetReused());
}

// -----------------------------------------------------------------------------
struct DeleteFunctor : public Functor<void, Agent*, AgentHandle> {
  std::vector<bool>& remove;

  DeleteFunctor(std::vector<bool>& remove) : remove(remove) {}
  virtual ~DeleteFunctor() {}

  void operator()(Agent* agent, AgentHandle ah) override {
    if (remove[agent->GetUid().GetIndex()]) {
      agent->RemoveFromSimulation();
    }
  }
};

// -----------------------------------------------------------------------------
void RunParallelAgentRemovalTest(
    uint64_t agents_per_dim,
    const std::function<bool(uint64_t index)>& remove_functor) {
  Simulation simulation("RunForEachAgentTest_ParallelAgentRemoval");

  auto construct = [](const Double3& pos) {
    auto* agent = new TestAgent(pos);
    agent->SetDiameter(10);
    return agent;
  };
  ModelInitializer::Grid3D(agents_per_dim, 20, construct);

  auto* rm = simulation.GetResourceManager();
  simulation.GetScheduler()->Simulate(1);

  std::vector<bool> remove(rm->GetNumAgents());
  for (uint64_t i = 0; i < remove.size(); ++i) {
    remove[i] = remove_functor(i);
  }

  DeleteFunctor f(remove);
  rm->ForEachAgentParallel(f);

  simulation.GetScheduler()->Simulate(1);

  for (uint64_t i = 0; i < remove.size(); ++i) {
    auto uid = AgentUid(i);
    EXPECT_EQ(!remove[i], rm->ContainsAgent(uid));
    if (!remove[i]) {
      // access data member to check that remaining agents
      // are still valid and haven't been deleted erroneously.
      EXPECT_EQ(uid, rm->GetAgent(uid)->GetUid());
    }
  }
}

// -----------------------------------------------------------------------------
TEST(ResourceManagerTest, ParallelAgentRemoval_SmallScale) {
  RunParallelAgentRemovalTest(
      2, [](uint64_t i) { return i == 0 || i == 3 || i == 6 || i == 7; });
}

// -----------------------------------------------------------------------------
TEST(ResourceManagerTest, ParallelAgentRemoval_SmallScale_All) {
  RunParallelAgentRemovalTest(2, [](uint64_t i) { return true; });
}

// -----------------------------------------------------------------------------
TEST(ResourceManagerTest, ParallelAgentRemoval_SmallScale_None) {
  RunParallelAgentRemovalTest(2, [](uint64_t i) { return false; });
}

// -----------------------------------------------------------------------------
TEST(ResourceManagerTest, ParallelAgentRemoval_SmallScale1) {
  RunParallelAgentRemovalTest(3, [](uint64_t i) {
    return i == 0 || i == 3 || (i >= 6 && i <= 11) || i == 13 || i == 14 ||
           (i >= 23 && i <= 26);
  });
}

// -----------------------------------------------------------------------------
TEST(ResourceManagerTest, ParallelAgentRemoval_LargeScale25) {
  RunParallelAgentRemovalTest(32, [](uint64_t i) {
    return Simulation::GetActive()->GetRandom()->Uniform() > 0.25;
  });
}

// -----------------------------------------------------------------------------
TEST(ResourceManagerTest, ParallelAgentRemoval_LargeScale50) {
  RunParallelAgentRemovalTest(32, [](uint64_t i) {
    return Simulation::GetActive()->GetRandom()->Uniform() > 0.5;
  });
}

// -----------------------------------------------------------------------------
TEST(ResourceManagerTest, ParallelAgentRemoval_LargeScale75) {
  RunParallelAgentRemovalTest(32, [](uint64_t i) {
    return Simulation::GetActive()->GetRandom()->Uniform() > 0.75;
  });
}

}  // namespace bdm
