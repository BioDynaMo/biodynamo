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
            const std::vector<Real3>& positions) {
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

// -----------------------------------------------------------------------------
// check that the ResourceManager contains a diffusion grid with matching
// boundaries
void VerifyDiffusionGrid(Simulation* sim, uint64_t substance_id,
                         double diffusion_coefficient, double decay_constant,
                         int resolution, BoundaryConditionType bct,
                         real_t boundary_value) {
  sim->GetExecutionContext()->SetupIterationAll(sim->GetAllExecCtxts());
  auto* rm = sim->GetResourceManager();

  // Get the diffusion grid
  auto* cm = rm->GetContinuum(substance_id);
  auto* dg = dynamic_cast<DiffusionGrid*>(cm);

  if (!dg) {
    Log::Fatal("ModelInitializerTest", "No diffusion grid found");
  }

  // Check the diffusion coefficient
  auto my_dc = dg->GetDiffusionCoefficients()[0];
  EXPECT_FLOAT_EQ(diffusion_coefficient, 1 - my_dc);

  // Check the decay constant
  auto my_decay = dg->GetDecayConstant();
  EXPECT_FLOAT_EQ(decay_constant, my_decay);

  // Check the resolution
  auto my_resolution = dg->GetResolution();
  EXPECT_EQ(resolution, my_resolution);

  // Check the boundary conditions
  auto my_bct = dg->GetBoundaryConditionType();
  EXPECT_EQ(bct, my_bct);

  // Check the boundary values
  auto* my_boudary = dg->GetBoundaryCondition();
  auto my_boundary_value = my_boudary->evaluate(0, 0, 0, 0);
  EXPECT_FLOAT_EQ(boundary_value, my_boundary_value);
}

// Tests if pos_0 cubic 3D grid of cells is correctly initialized
TEST(ModelInitializerTest, Grid3DCube) {
  Simulation simulation(TEST_NAME);

  ModelInitializer::Grid3D(2, 12, [](const Real3& pos) {
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

  ModelInitializer::Grid3D(grid_dimensions, 12, [](const Real3& pos) {
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

  std::vector<Real3> positions;
  positions.push_back({1, 2, 3});
  positions.push_back({101, 202, 303});
  positions.push_back({-12, -32, 4});

  ModelInitializer::CreateAgents(positions, [](const Real3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  Verify(&simulation, 3u, {{1, 2, 3}, {101, 202, 303}, {-12, -32, 4}});
}

TEST(ModelInitializerTest, CreateAgentsRandom) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  ModelInitializer::CreateAgentsRandom(-100, 100, 10, [](const Real3& pos) {
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
  Real3 center{1.0, 2.0, 3.0};
  real_t radius{10.0};
  uint64_t no_agents{100};

  ModelInitializer::CreateAgentsInSphereRndm(center, radius, no_agents,
                                             [](const Real3& pos) {
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

TEST(ModelInitializerTest, DGandBoundaries) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  rm->AddAgent(new Cell({0, 0, 0}));

  real_t diffusion_coefficient_0 = 1.0;
  real_t diffusion_coefficient_1 = 2.0;
  real_t decay_constant_0 = 0.1;
  real_t decay_constant_1 = 0.2;
  int resolution_0 = 10;
  int resolution_1 = 20;
  real_t boundary_value_0 = 1.0;
  real_t boundary_value_1 = -1.0;
  auto boundary_condition_0 =
      std::make_unique<ConstantBoundaryCondition>(boundary_value_0);
  auto boundary_condition_1 =
      std::make_unique<ConstantBoundaryCondition>(boundary_value_1);

  ModelInitializer::DefineSubstance(0, "substance_0", diffusion_coefficient_0,
                                    decay_constant_0, resolution_0);
  ModelInitializer::DefineSubstance(1, "substance_1", diffusion_coefficient_1,
                                    decay_constant_1, resolution_1);

  ModelInitializer::AddBoundaryConditions(0, BoundaryConditionType::kDirichlet,
                                          std::move(boundary_condition_0));
  ModelInitializer::AddBoundaryConditions(1, BoundaryConditionType::kNeumann,
                                          std::move(boundary_condition_1));

  VerifyDiffusionGrid(&simulation, 0, diffusion_coefficient_0, decay_constant_0,
                      resolution_0, BoundaryConditionType::kDirichlet, 1.0);
  VerifyDiffusionGrid(&simulation, 1, diffusion_coefficient_1, decay_constant_1,
                      resolution_1, BoundaryConditionType::kNeumann, -1.0);

  // Check that the boundary conditions are now set to nullptr
  EXPECT_EQ(nullptr, boundary_condition_0.get());
  EXPECT_EQ(nullptr, boundary_condition_0.get());
}

}  // namespace model_initializer_test_internal
}  // namespace bdm
