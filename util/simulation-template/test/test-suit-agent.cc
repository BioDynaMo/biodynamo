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

#include <gtest/gtest.h>
#include "biodynamo.h"

// Googletest in combination with the provided CMakeLists.txt allows you to
// define tests in arbitrary .cc files in the `test/` folder. We propose
// this file to test the agent. The examples below are not necessary because
// they are tested in BioDynaMo itself. It should serve as an inspiration for
// testing user-defined, customn behaviours or similar things. If you wish to
// add tests for other aspects, you can either add them to the existing
// test-*.cc files or create a new test-*.cc file in the test/folder. CMake will
// handle it automatically.

#define TEST_NAME typeid(*this).name()

namespace bdm {

// Test if we can add agents to the simulation
TEST(AgentTest, AddAgentsToSimulation) {
  // Create simulation
  Simulation simulation(TEST_NAME);

  // Add some cells to the simulation
  auto* rm = simulation.GetResourceManager();
  uint8_t expected_no_cells{20};
  for (int i = 0; i < expected_no_cells; i++) {
    auto* cell = new Cell(30);
    rm->AddAgent(cell);
  }

  // Test if all 20 cells are in the simulation
  auto no_cells = rm->GetNumAgents();
  EXPECT_EQ(expected_no_cells, no_cells);
}

// Test if our GrothDevision behaviour increases the cell volume as expected
TEST(AgentTest, CellGrowth) {
  // Create simulation
  Simulation simulation(TEST_NAME);

  // Add one growing cell to the simulation
  auto* rm = simulation.GetResourceManager();
  auto* cell = new Cell(30);
  cell->AddBehavior(new GrowthDivision());
  const AgentPointer<Cell> cell_ptr = cell->GetAgentPtr<Cell>();
  rm->AddAgent(cell);

  // Get cell volume at the beginning
  const double volume_1 = cell_ptr->GetVolume();

  // Simulate for 3 timesteps
  simulation.Simulate(3);

  // Get cell volume at the beginning after 3 timesteps
  const double volume_2 = cell_ptr->GetVolume();

  // Test if cell volume has increased
  EXPECT_LT(volume_1, volume_2);
}

}  // namespace bdm