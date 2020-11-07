// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "unit/core/agent/agent_test.h"
#include "core/resource_manager.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace agent_test_internal {

TEST(AgentTest, CopyCtor) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;
  cell.SetBoxIdx(123);
  GrowthModule* gm = new GrowthModule();
  gm->growth_rate_ = 321;
  cell.AddBehavior(gm);

  TestAgent copy(cell);
  EXPECT_EQ(123u, copy.GetBoxIdx());
  EXPECT_EQ(cell.GetUid(), copy.GetUid());
  ASSERT_EQ(1u, copy.GetAllBehaviors().size());
  GrowthModule* copy_gm =
      dynamic_cast<GrowthModule*>(copy.GetAllBehaviors()[0]);
  EXPECT_TRUE(gm != copy_gm);
  EXPECT_EQ(321, copy_gm->growth_rate_);
}

TEST(AgentTest, Behavior) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;
  double diameter = cell.GetDiameter();
  auto position = cell.GetPosition();

  cell.AddBehavior(new MovementModule({1, 2, 3}));
  cell.AddBehavior(new GrowthModule());

  cell.RunBehaviors();

  EXPECT_NEAR(diameter + 0.5, cell.GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(position[0] + 1, cell.GetPosition()[0], abs_error<double>::value);
  EXPECT_NEAR(position[1] + 2, cell.GetPosition()[1], abs_error<double>::value);
  EXPECT_NEAR(position[2] + 3, cell.GetPosition()[2], abs_error<double>::value);
}

TEST(AgentTest, GetBehaviorsTest) {
  Simulation simulation(TEST_NAME);

  // create cell and add behaviors
  TestAgent cell;
  cell.AddBehavior(new GrowthModule());
  cell.AddBehavior(new GrowthModule());
  cell.AddBehavior(new MovementModule({1, 2, 3}));

  uint64_t growth_module_cnt = 0;
  uint64_t movement_module_cnt = 0;
  for (auto* behavior : cell.GetAllBehaviors()) {
    if (dynamic_cast<GrowthModule*>(behavior)) {
      growth_module_cnt++;
    } else if (MovementModule* mm = dynamic_cast<MovementModule*>(behavior)) {
      movement_module_cnt++;
      EXPECT_ARR_NEAR(mm->velocity_, {1, 2, 3});
    }
  }

  EXPECT_EQ(2u, growth_module_cnt);
  EXPECT_EQ(1u, movement_module_cnt);
}

TEST(AgentTest, BehaviorEventHandler) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;

  cell.AddBehavior(new MovementModule({1, 2, 3}));
  cell.AddBehavior(new GrowthModule());

  CellDivisionEvent event(1, 2, 3);
  TestAgent copy(event, &cell, 0);
  cell.EventHandler(event, &copy);

  const auto& bms = cell.GetAllBehaviors();
  ASSERT_EQ(1u, bms.size());
  EXPECT_TRUE(dynamic_cast<GrowthModule*>(bms[0]) != nullptr);

  const auto& copy_behaviors = copy.GetAllBehaviors();
  ASSERT_EQ(1u, copy_behaviors.size());
  EXPECT_TRUE(dynamic_cast<GrowthModule*>(copy_behaviors[0]) != nullptr);
}

TEST(AgentTest, RemoveBehavior) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;

  // add RemoveModule as first one! If removal while iterating over it is not
  // implemented correctly, MovementModule will not be executed.
  cell.AddBehavior(new RemoveModule());
  cell.AddBehavior(new MovementModule({1, 2, 3}));
  cell.AddBehavior(new GrowthModule());

  // RemoveModule should remove itself
  cell.RunBehaviors();

  const auto& bms = cell.GetAllBehaviors();
  ASSERT_EQ(2u, bms.size());
  EXPECT_TRUE(dynamic_cast<MovementModule*>(bms[0]) != nullptr);
  EXPECT_TRUE(dynamic_cast<GrowthModule*>(bms[1]) != nullptr);
  // check if MovementModule and GrowthModule have been executed correctly.
  EXPECT_ARR_NEAR({1, 2, 3}, cell.GetPosition());
  EXPECT_NEAR(10.5, cell.GetDiameter(), abs_error<double>::value);

  cell.AddBehavior(new RemoveModule());
  ASSERT_EQ(3u, bms.size());
  auto* to_be_removed = dynamic_cast<RemoveModule*>(bms[2]);
  cell.RemoveBehavior(to_be_removed);
  ASSERT_EQ(2u, bms.size());
}

TEST(AgentTest, GetAgentPtr) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  std::vector<Agent*> agents;
  for (uint64_t i = 0; i < 10; i++) {
    auto* agent = new TestAgent();
    rm->AddAgent(agent);
    agents.push_back(agent);
  }
  EXPECT_EQ(10u, rm->GetNumAgents());

  for (uint64_t i = 0; i < 10; i++) {
    AgentPointer<TestAgent> expected(agents[i]->GetUid());
    EXPECT_EQ(expected, agents[i]->GetAgentPtr<TestAgent>());

    AgentPointer<Agent> expected1(agents[i]->GetUid());
    EXPECT_EQ(expected1, agents[i]->GetAgentPtr());
  }
}

}  // namespace agent_test_internal
}  // namespace bdm
