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

#include "core/model_initializer.h"
#include "core/agent/cell.h"
#include "core/behavior/behavior.h"
#include "core/resource_manager.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace model_initializer_test_internal {

// -----------------------------------------------------------------------------
// check that the ResourceManager contains one agent with matching
// each position.
void Verify(Simulation* sim, uint64_t num_agents,
            const std::vector<Double3>& positions) {
  sim->GetExecutionContext()->SetupIterationAll(sim->GetAllExecCtxts());
  auto* rm = sim->GetResourceManager();

  ASSERT_EQ(num_agents, rm->GetNumAgents());

  for (auto& pos : positions) {
    uint64_t cnt = 0;
    rm->ForEachAgent([&](Agent* agent, AgentHandle) {
      auto diff = pos - agent->GetPosition();
      if (diff.Norm() < 1e-5) {
        cnt++;
      }
    });
    EXPECT_EQ(1u, cnt);
  }
}

// Tests if pos_0 cubic 3D grid of cells is correctly initialized
TEST(ModelInitializerTest, Grid3DCube) {
  Simulation simulation(TEST_NAME);

  ModelInitializer::Grid3D(2, 12, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  Verify(&simulation, 8u,
         {{0, 0, 0},
          {0, 0, 12},
          {0, 12, 0},
          {0, 12, 12},
          {12, 0, 0},
          {12, 0, 12},
          {12, 12, 0},
          {12, 12, 12}});
}

// Tests if pos_0 cuboid 3D grid of cells is correctly initialized
TEST(ModelInitializerTest, Grid3DCuboid) {
  Simulation simulation(TEST_NAME);

  std::array<size_t, 3> grid_dimensions = {2, 3, 4};

  ModelInitializer::Grid3D(grid_dimensions, 12, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  Verify(&simulation, 24u,
         {{0, 0, 0},
          {0, 0, 12},
          {0, 0, 24},
          {0, 0, 36},
          {0, 12, 0},
          {0, 12, 12},
          {0, 12, 24},
          {12, 24, 36}});
}

TEST(ModelInitializerTest, CreateAgents) {
  Simulation simulation(TEST_NAME);

  std::vector<Double3> positions;
  positions.push_back({1, 2, 3});
  positions.push_back({101, 202, 303});
  positions.push_back({-12, -32, 4});

  ModelInitializer::CreateAgents(positions, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  Verify(&simulation, 3u, {{1, 2, 3}, {101, 202, 303}, {-12, -32, 4}});
}

TEST(ModelInitializerTest, CreateAgentsRandom) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  ModelInitializer::CreateAgentsRandom(-100, 100, 10, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  simulation.GetExecutionContext()->SetupIterationAll(
      simulation.GetAllExecCtxts());

  EXPECT_EQ(10u, rm->GetNumAgents());
  auto& pos_0 = rm->GetAgent(AgentUid(0))->GetPosition();
  auto& pos_1 = rm->GetAgent(AgentUid(1))->GetPosition();
  auto& pos_2 = rm->GetAgent(AgentUid(2))->GetPosition();
  EXPECT_TRUE((pos_0[0] >= -100) && (pos_0[0] <= 100));
  EXPECT_TRUE((pos_0[1] >= -100) && (pos_0[1] <= 100));
  EXPECT_TRUE((pos_0[2] >= -100) && (pos_0[2] <= 100));

  EXPECT_TRUE((pos_1[0] >= -100) && (pos_1[0] <= 100));
  EXPECT_TRUE((pos_1[1] >= -100) && (pos_1[1] <= 100));
  EXPECT_TRUE((pos_1[2] >= -100) && (pos_1[2] <= 100));

  EXPECT_TRUE((pos_2[0] >= -100) && (pos_2[0] <= 100));
  EXPECT_TRUE((pos_2[1] >= -100) && (pos_2[1] <= 100));
  EXPECT_TRUE((pos_2[2] >= -100) && (pos_2[2] <= 100));
}

// This test checks if CreateAgentsInSphereRndm creates the correct amount of
// agents in a distance no further than `r` from a specified center. It does not
// test if the points are uniformly distributed.
TEST(ModelInitializerTest, CreateAgentsInSphereRndm) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  Double3 center{1.0, 2.0, 3.0};
  double radius{10.0};
  uint64_t no_agents{100};

  ModelInitializer::CreateAgentsInSphereRndm(center, radius, no_agents,
                                             [](const Double3& pos) {
                                               Cell* cell = new Cell(pos);
                                               return cell;
                                             });

  simulation.GetExecutionContext()->SetupIterationAll(
      simulation.GetAllExecCtxts());

  EXPECT_EQ(100u, rm->GetNumAgents());
  rm->ForEachAgent([&](Agent* agent) {
    Cell* cell = bdm_static_cast<Cell*>(agent);
    auto shift = cell->GetPosition() - center;
    EXPECT_LE(shift.Norm(), radius);
  });
}

}  // namespace model_initializer_test_internal
}  // namespace bdm
