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

#include "unit/core/agent/agent_test.h"
#include "core/behavior/stateless_behavior.h"
#include "core/environment/environment.h"
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
  real diameter = cell.GetDiameter();
  auto position = cell.GetPosition();

  cell.AddBehavior(new Movement({1, 2, 3}));
  cell.AddBehavior(new Growth());

  cell.RunBehaviors();

  EXPECT_NEAR(diameter + 0.5, cell.GetDiameter(), abs_error<real>::value);
  EXPECT_NEAR(position[0] + 1, cell.GetPosition()[0], abs_error<real>::value);
  EXPECT_NEAR(position[1] + 2, cell.GetPosition()[1], abs_error<real>::value);
  EXPECT_NEAR(position[2] + 3, cell.GetPosition()[2], abs_error<real>::value);
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
  EXPECT_NEAR(10.5, cell.GetDiameter(), abs_error<real>::value);

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
TEST(AgentTest, StaticnessOff) {
  // Agent::IsStatic should always be false
  auto set_param = [](Param* param) { param->detect_static_agents = false; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  // no interference from mechanical forces operation
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  std::unordered_map<AgentUid, bool> static_agents_map;

  auto* agent = new Cell();
  agent->SetDiameter(10);
  agent->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr = agent->GetAgentPtr<Cell>();
  auto auid = agent->GetUid();

  EXPECT_FALSE(agent->IsStatic());

  rm->AddAgent(agent);

  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  StatelessBehavior grow(
      [](Agent* agent) { agent->SetDiameter(agent->GetDiameter() + 5); });
  aptr->AddBehavior(grow.NewCopy());

  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  aptr->RemoveBehavior(aptr->GetAllBehaviors()[1]);
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  aptr->SetDiameter(20);
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  StatelessBehavior shrink(
      [](Agent* agent) { agent->SetDiameter(agent->GetDiameter() - 1); });
  aptr->AddBehavior(shrink.NewCopy());

  // should be true because the diameter was not growing
  scheduler->Simulate(2);
  EXPECT_FALSE(static_agents_map[auid]);
}

// -----------------------------------------------------------------------------
TEST(AgentTest, StaticnessBasic) {
  auto set_param = [](Param* param) { param->detect_static_agents = true; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  // no interference from mechanical forces operation
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  std::unordered_map<AgentUid, bool> static_agents_map;

  auto* agent = new Cell();
  agent->SetDiameter(10);
  agent->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr = agent->GetAgentPtr<Cell>();
  auto auid = agent->GetUid();

  // should be false right after creation
  EXPECT_FALSE(agent->IsStatic());

  rm->AddAgent(agent);

  // should be false in the first iteration
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  // should be true in its second iteration since it hasn't been moved
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
  aptr->SetDiameter(20);
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);

  StatelessBehavior shrink(
      [](Agent* agent) { agent->SetDiameter(agent->GetDiameter() - 1); });
  aptr->AddBehavior(shrink.NewCopy());

  // should be true because the diameter was not growing
  scheduler->Simulate(2);
  EXPECT_TRUE(static_agents_map[auid]);
}

// -----------------------------------------------------------------------------
TEST(AgentTest, StaticnessNeighbors) {
  auto set_param = [](Param* param) { param->detect_static_agents = true; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  // no interference from mechanical forces operation
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  std::unordered_map<AgentUid, bool> static_agents_map;

  auto* agent = new Cell({0, 0, 0});
  agent->SetDiameter(10);
  agent->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr = agent->GetAgentPtr<Cell>();
  auto auid = agent->GetUid();

  auto* neighbor = new Cell({10, 0, 0});
  neighbor->SetDiameter(10);
  neighbor->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto nptr = neighbor->GetAgentPtr<Cell>();
  auto nuid = neighbor->GetUid();

  // should be false right after creation
  EXPECT_FALSE(agent->IsStatic());
  EXPECT_FALSE(neighbor->IsStatic());

  rm->AddAgent(agent);
  rm->AddAgent(neighbor);

  // should be false in the first iteration
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);
  EXPECT_FALSE(static_agents_map[nuid]);

  // should be true in its second iteration since it hasn't been moved
  scheduler->Simulate(1);
  EXPECT_TRUE(static_agents_map[auid]);
  EXPECT_TRUE(static_agents_map[nuid]);

  // simulate modification from neighbor -> should be false
  aptr->SetDiameter(20);

  auto* env = simulation.GetEnvironment();
  env->ForcedUpdate();
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid]);
  EXPECT_FALSE(static_agents_map[nuid]);
}

// -----------------------------------------------------------------------------
TEST(AgentTest, StaticnessNewAgent) {
  auto set_param = [](Param* param) { param->detect_static_agents = true; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  // no interference from mechanical forces operation
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  std::unordered_map<AgentUid, bool> static_agents_map;

  auto* agent1 = new Cell({0, 0, 0});
  agent1->SetDiameter(10);
  agent1->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr1 = agent1->GetAgentPtr<Cell>();
  auto auid1 = agent1->GetUid();

  auto* agent2 = new Cell({10, 0, 0});
  agent2->SetDiameter(10);
  agent2->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr2 = agent2->GetAgentPtr<Cell>();
  auto auid2 = agent2->GetUid();

  auto* agent3 = new Cell({30, 0, 0});
  agent3->SetDiameter(10);
  agent3->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto auid3 = agent3->GetUid();

  auto* agent4 = new Cell({40, 0, 0});
  agent4->SetDiameter(10);
  agent4->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto auid4 = agent4->GetUid();

  // should be false right after creation
  EXPECT_FALSE(agent1->IsStatic());
  EXPECT_FALSE(agent2->IsStatic());
  EXPECT_FALSE(agent3->IsStatic());
  EXPECT_FALSE(agent4->IsStatic());

  rm->AddAgent(agent1);
  rm->AddAgent(agent2);
  rm->AddAgent(agent3);
  rm->AddAgent(agent4);

  // should be false in the first iteration
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid1]);
  EXPECT_FALSE(static_agents_map[auid2]);
  EXPECT_FALSE(static_agents_map[auid3]);
  EXPECT_FALSE(static_agents_map[auid4]);

  // should be true in its second iteration since it hasn't been moved
  scheduler->Simulate(1);
  EXPECT_TRUE(static_agents_map[auid1]);
  EXPECT_TRUE(static_agents_map[auid2]);
  EXPECT_TRUE(static_agents_map[auid3]);
  EXPECT_TRUE(static_agents_map[auid4]);

  // create new agent
  auto* new_agent = new Cell({20, 0, 0});
  //   big enough to overlap with agent1 and agent2
  new_agent->SetDiameter(12);
  new_agent->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto naptr = new_agent->GetAgentPtr<Cell>();
  auto nauid = new_agent->GetUid();
  rm->AddAgent(new_agent);

  scheduler->Simulate(1);
  EXPECT_TRUE(static_agents_map[auid1]);
  EXPECT_TRUE(static_agents_map[auid4]);
  EXPECT_FALSE(static_agents_map[auid2]);
  EXPECT_FALSE(static_agents_map[auid3]);
  EXPECT_FALSE(static_agents_map[nauid]);
}

// -----------------------------------------------------------------------------
TEST(AgentTest, StaticnessNewAgentLargetThanAllOthers) {
  // This test tests if static agent detection works even
  // if an agent is created that is larger than all others.
  // This could be problematic if implemented incorrectly,
  // because the uniform grid environment can only return
  // agents within its box length.
  auto set_param = [](Param* param) { param->detect_static_agents = true; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  // no interference from mechanical forces operation
  scheduler->UnscheduleOp(scheduler->GetOps("mechanical forces")[0]);

  std::unordered_map<AgentUid, bool> static_agents_map;

  auto* agent1 = new Cell({0, 0, 0});
  agent1->SetDiameter(10);
  agent1->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr1 = agent1->GetAgentPtr<Cell>();
  auto auid1 = agent1->GetUid();

  auto* agent2 = new Cell({10, 0, 0});
  agent2->SetDiameter(10);
  agent2->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto aptr2 = agent2->GetAgentPtr<Cell>();
  auto auid2 = agent2->GetUid();

  auto* agent3 = new Cell({30, 0, 0});
  agent3->SetDiameter(10);
  agent3->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto auid3 = agent3->GetUid();

  auto* agent4 = new Cell({40, 0, 0});
  agent4->SetDiameter(10);
  agent4->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto auid4 = agent4->GetUid();

  // should be false right after creation
  EXPECT_FALSE(agent1->IsStatic());
  EXPECT_FALSE(agent2->IsStatic());
  EXPECT_FALSE(agent3->IsStatic());
  EXPECT_FALSE(agent4->IsStatic());

  rm->AddAgent(agent1);
  rm->AddAgent(agent2);
  rm->AddAgent(agent3);
  rm->AddAgent(agent4);

  // should be false in the first iteration
  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid1]);
  EXPECT_FALSE(static_agents_map[auid2]);
  EXPECT_FALSE(static_agents_map[auid3]);
  EXPECT_FALSE(static_agents_map[auid4]);

  // should be true in its second iteration since it hasn't been moved
  scheduler->Simulate(1);
  EXPECT_TRUE(static_agents_map[auid1]);
  EXPECT_TRUE(static_agents_map[auid2]);
  EXPECT_TRUE(static_agents_map[auid3]);
  EXPECT_TRUE(static_agents_map[auid4]);

  // create new agent
  auto* new_agent = new Cell({20, 0, 0});
  //   big enough to overlap with all agents
  new_agent->SetDiameter(22);
  new_agent->AddBehavior(new CaptureStaticness(&static_agents_map));
  auto nauid = new_agent->GetUid();
  rm->AddAgent(new_agent);

  scheduler->Simulate(1);
  EXPECT_FALSE(static_agents_map[auid1]);
  EXPECT_FALSE(static_agents_map[auid2]);
  EXPECT_FALSE(static_agents_map[auid3]);
  EXPECT_FALSE(static_agents_map[auid4]);
  EXPECT_FALSE(static_agents_map[nauid]);
}

}  // namespace agent_test_internal
}  // namespace bdm
