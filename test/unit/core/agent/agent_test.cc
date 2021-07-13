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

#include "unit/core/agent/agent_test.h"
#include "core/behavior/stateless_behavior.h"
#include "core/resource_manager.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace agent_test_internal {

TEST(AgentTest, CopyCtor) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;
  cell.SetBoxIdx(123);
  Growth* g = new Growth();
  g->growth_rate_ = 321;
  cell.AddBehavior(g);

  TestAgent copy(cell);
  EXPECT_EQ(123u, copy.GetBoxIdx());
  EXPECT_EQ(cell.GetUid(), copy.GetUid());
  ASSERT_EQ(1u, copy.GetAllBehaviors().size());
  Growth* copy_g = dynamic_cast<Growth*>(copy.GetAllBehaviors()[0]);
  EXPECT_TRUE(g != copy_g);
  EXPECT_EQ(321, copy_g->growth_rate_);
}

TEST(AgentTest, Behavior) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;
  double diameter = cell.GetDiameter();
  auto position = cell.GetPosition();

  cell.AddBehavior(new Movement({1, 2, 3}));
  cell.AddBehavior(new Growth());

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
  cell.AddBehavior(new Growth());
  cell.AddBehavior(new Growth());
  cell.AddBehavior(new Movement({1, 2, 3}));

  uint64_t growth_cnt = 0;
  uint64_t movement_cnt = 0;
  for (auto* behavior : cell.GetAllBehaviors()) {
    if (dynamic_cast<Growth*>(behavior)) {
      growth_cnt++;
    } else if (Movement* m = dynamic_cast<Movement*>(behavior)) {
      movement_cnt++;
      EXPECT_ARR_NEAR(m->velocity_, {1, 2, 3});
    }
  }

  EXPECT_EQ(2u, growth_cnt);
  EXPECT_EQ(1u, movement_cnt);
}

TEST(AgentTest, BehaviorUpdate) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;

  cell.AddBehavior(new Movement({1, 2, 3}));
  cell.AddBehavior(new Growth());

  CellDivisionEvent event(1, 2, 3);
  event.existing_agent = &cell;
  TestAgent copy;
  copy.Initialize(event);
  cell.Update(event);

  const auto& behaviors = cell.GetAllBehaviors();
  ASSERT_EQ(1u, behaviors.size());
  EXPECT_TRUE(dynamic_cast<Growth*>(behaviors[0]) != nullptr);

  const auto& copy_behaviors = copy.GetAllBehaviors();
  ASSERT_EQ(1u, copy_behaviors.size());
  EXPECT_TRUE(dynamic_cast<Growth*>(copy_behaviors[0]) != nullptr);
}

TEST(AgentTest, RemoveSingleBehavior) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;

  cell.AddBehavior(new Growth());
  ASSERT_EQ(1u, cell.GetAllBehaviors().size());
  cell.RemoveBehavior(cell.GetAllBehaviors()[0]);
  ASSERT_EQ(0u, cell.GetAllBehaviors().size());
}

TEST(AgentTest, RemoveBehavior) {
  Simulation simulation(TEST_NAME);

  TestAgent cell;

  // add Removal as first one! If removal while iterating over it is not
  // implemented correctly, Movement will not be executed.
  cell.AddBehavior(new Removal());
  cell.AddBehavior(new Movement({1, 2, 3}));
  cell.AddBehavior(new Growth());

  // Removal should remove itself
  cell.RunBehaviors();

  const auto& behaviors = cell.GetAllBehaviors();
  ASSERT_EQ(2u, behaviors.size());
  EXPECT_TRUE(dynamic_cast<Movement*>(behaviors[0]) != nullptr);
  EXPECT_TRUE(dynamic_cast<Growth*>(behaviors[1]) != nullptr);
  // check if Movement and Growth have been executed correctly.
  EXPECT_ARR_NEAR({1, 2, 3}, cell.GetPosition());
  EXPECT_NEAR(10.5, cell.GetDiameter(), abs_error<double>::value);

  cell.AddBehavior(new Removal());
  ASSERT_EQ(3u, behaviors.size());
  auto* to_be_removed = dynamic_cast<Removal*>(behaviors[2]);
  cell.RemoveBehavior(to_be_removed);
  ASSERT_EQ(2u, behaviors.size());
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

// -----------------------------------------------------------------------------
TEST(AgentTest, StaticnessBasic) {
  auto set_param = [](Param* param) {
    param->detect_static_agents = true;
    param->show_simulation_step = true;
  };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  scheduler->GetOps("load balancing")[0]->frequency_ = 0;

  std::unordered_map<AgentUid, bool> static_agents_map;

  auto* agent = new Cell();
  agent->SetDiameter(10);
  agent->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr = agent->GetAgentPtr<TestAgent>();
  auto auid = agent->GetUid();

  // should be false right after creation
  EXPECT_FALSE(agent->IsStatic());

  rm->AddAgent(agent);

  // should be false in its first two iterations
  // the default value of is_static_ is true and
  // also propagate_staticness_neighborhood_ is true
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  // should be true in its third iteration since it hasn't been moved
  scheduler->Simulate(1);
  EXPECT_TRUE(static_agents_map[auid]);

  StatelessBehavior grow(
      [](Agent* agent) { agent->SetDiameter(agent->GetDiameter() + 5); });
  aptr->AddBehavior(grow.NewCopy());

  // should be true because the diameter is growing in the same iteration
  scheduler->Simulate(1);
  EXPECT_TRUE(static_agents_map[auid]);

  aptr->RemoveBehavior(aptr->GetAllBehaviors()[1]);
  // should be false because the diameter was growing in the previous iteration
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  // should be true because the diameter was not growing in the previous
  // iteration
  scheduler->Simulate(1);
  EXPECT_TRUE(static_agents_map[auid]);

  // simulate modification from neighbor -> should be false
  agent->SetDiameter(20);
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  StatelessBehavior shrink(
      [](Agent* agent) { agent->SetDiameter(agent->GetDiameter() - 1); });
  aptr->AddBehavior(shrink.NewCopy());

  // should be true because the diameter was not growing
  scheduler->Simulate(2);
  EXPECT_TRUE(static_agents_map[auid]);
}

}  // namespace agent_test_internal
}  // namespace bdm
