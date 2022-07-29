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

#include "core/simulation_space.h"
#include <gtest/gtest.h>
#include "unit/test_util/test_util.h"
#include "core/agent/cell.h"
#include "core/resource_manager.h"

namespace bdm {
namespace simulation_space_test {

// -----------------------------------------------------------------------------
TEST(SimulationSpace, EmptyUseParam) {
  // tests a fixed space
  auto set_param = [](Param* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 10;
    param->max_bound = 20;
    param->interaction_radius = 3;
  };

  Simulation simulation(TEST_NAME, set_param);

  auto* space = simulation.GetSimulationSpace();
  MathArray<int32_t, 6> expected_space = {10, 20, 10, 20, 10, 20};
  EXPECT_EQ(expected_space, space->GetWholeSpace());
  EXPECT_EQ(expected_space, space->GetLocalSpace());

  EXPECT_EQ(3, space->GetInteractionRadius());
  EXPECT_EQ(9, space->GetInteractionRadiusSquared());

  // A call to Update must not change the state
  space->Update();
  EXPECT_EQ(expected_space, space->GetWholeSpace());
  EXPECT_EQ(expected_space, space->GetLocalSpace());

  EXPECT_EQ(3, space->GetInteractionRadius());
  EXPECT_EQ(9, space->GetInteractionRadiusSquared());
}

// -----------------------------------------------------------------------------
TEST(SimulationSpace, EmptyUseMemberFunctions) {
  // tests a fixed space
  Simulation simulation(TEST_NAME);

  auto* space = simulation.GetSimulationSpace();
  space->SetWholeSpace({10, 20, 10, 20, 10, 20});
  space->SetInteractionRadius(3);

  MathArray<int32_t, 6> expected_space = {10, 20, 10, 20, 10, 20};
  EXPECT_EQ(expected_space, space->GetWholeSpace());
  EXPECT_EQ(expected_space, space->GetLocalSpace());

  EXPECT_EQ(3, space->GetInteractionRadius());
  EXPECT_EQ(9, space->GetInteractionRadiusSquared());

  // A call to Update must not change the state
  space->Update();
  EXPECT_EQ(expected_space, space->GetWholeSpace());
  EXPECT_EQ(expected_space, space->GetLocalSpace());

  EXPECT_EQ(3, space->GetInteractionRadius());
  EXPECT_EQ(9, space->GetInteractionRadiusSquared());
}

// -----------------------------------------------------------------------------
TEST(SimulationSpaceDeathTest, EmptyNoFixedSpace) {
  ASSERT_DEATH(
      {
        Simulation simulation(TEST_NAME);

        auto* space = simulation.GetSimulationSpace();
        space->SetInteractionRadius(3);

        // call to update must result in fatal error, because
        // SimulationSpace is not able to determine the space
        space->Update();
      },
      "");
}

// -----------------------------------------------------------------------------
TEST(SimulationSpaceDeathTest, EmptyNoInteractionRadius) {
  ASSERT_DEATH(
      {
        Simulation simulation(TEST_NAME);

        auto* space = simulation.GetSimulationSpace();
        space->SetWholeSpace({10, 20, 10, 20, 10, 20});

        // call to update must result in fatal error, because
        // SimulationSpace is not able to determine the space
        space->Update();
      },
      "");
}

// -----------------------------------------------------------------------------
void CellFactory(ResourceManager* rm, size_t cells_per_dim) {
  const real_t space = 20;
  rm->Reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell* cell = new Cell({k * space, j * space, i * space});
        cell->SetDiameter(30);
        rm->AddAgent(cell);
      }
    }
  }
}

// -----------------------------------------------------------------------------
TEST(SimulationSpace, AutomaticSpaceAndInteractionRadius) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* space = simulation.GetSimulationSpace();

  CellFactory(rm, 3);

  space->Update();

  MathArray<int32_t, 6> expected_space = {{0, 40, 0, 40, 0, 40}};

  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(30, space->GetInteractionRadius());
  EXPECT_EQ(900, space->GetInteractionRadiusSquared());

  // modify one agent
  rm->GetAgent(AgentUid(0))->SetPosition({{100, -12, 0}});
  rm->GetAgent(AgentUid(0))->SetDiameter(40);
  space->Update();

  // check if the values have been updated based on the modified agent
  // attributes
  expected_space = {{0, 100, -12, 40, 0, 40}};
  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(40, space->GetInteractionRadius());
  EXPECT_EQ(1600, space->GetInteractionRadiusSquared());
}

// -----------------------------------------------------------------------------
TEST(SimulationSpace, AutomaticSpaceFixedInteractionRadius) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* space = simulation.GetSimulationSpace();
  space->SetInteractionRadius(10);

  CellFactory(rm, 3);

  space->Update();

  MathArray<int32_t, 6> expected_space = {{0, 40, 0, 40, 0, 40}};

  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(10, space->GetInteractionRadius());
  EXPECT_EQ(100, space->GetInteractionRadiusSquared());

  // modify one agent
  rm->GetAgent(AgentUid(0))->SetPosition({{100, -12, 0}});
  rm->GetAgent(AgentUid(0))->SetDiameter(40);
  space->Update();

  // check if the values have been updated based on the modified agent
  // attributes
  expected_space = {{0, 100, -12, 40, 0, 40}};
  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(10, space->GetInteractionRadius());
  EXPECT_EQ(100, space->GetInteractionRadiusSquared());
}

// -----------------------------------------------------------------------------
TEST(SimulationSpace, FixedSpaceAutomaticInteractionRadius) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* space = simulation.GetSimulationSpace();
  space->SetWholeSpace({-21, 32, -21, 32, -21, 32});

  CellFactory(rm, 3);

  space->Update();

  MathArray<int32_t, 6> expected_space = {{-21, 32, -21, 32, -21, 32}};

  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(30, space->GetInteractionRadius());
  EXPECT_EQ(900, space->GetInteractionRadiusSquared());

  // modify one agent
  rm->GetAgent(AgentUid(0))->SetPosition({{100, -12, 0}});
  rm->GetAgent(AgentUid(0))->SetDiameter(40);
  space->Update();

  // check if the values have been updated based on the modified agent
  // attributes
  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(40, space->GetInteractionRadius());
  EXPECT_EQ(1600, space->GetInteractionRadiusSquared());
}

// -----------------------------------------------------------------------------
TEST(SimulationSpace, AutomaticSpaceAndInteractionRadius_EmptySim) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* space = simulation.GetSimulationSpace();

  CellFactory(rm, 3);

  space->Update();

  MathArray<int32_t, 6> expected_space = {{0, 40, 0, 40, 0, 40}};

  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(30, space->GetInteractionRadius());
  EXPECT_EQ(900, space->GetInteractionRadiusSquared());

  // remove all agents
  rm->ClearAgents();

  // values should not have changed
  EXPECT_EQ(expected_space, space->GetLocalSpace());
  EXPECT_EQ(30, space->GetInteractionRadius());
  EXPECT_EQ(900, space->GetInteractionRadiusSquared());
}

}  // namespace simulation_space_test
}  // namespace bdm
