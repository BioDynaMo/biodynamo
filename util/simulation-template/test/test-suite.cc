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

#include <gtest/gtest.h>
#include "biodynamo.h"

// Googletest in combination with the provided CMakeLists.txt allows you to
// define tests in arbitrary .cc files in the `test/` folder. This file should
// serve as an inspiration for testing user-defined, custom behaviors, basic as
// well as compicated functions, or similar things. If you wish to add tests for
// specific aspects, you can either add them to the existing test-suite.cc file
// or create a new *.cc file in the `test/` folder. CMake will handle it
// automatically. For more information regarding testing with Googletest, please
// consider the following sources:
// * https://google.github.io/googletest/primer.html
// * https://github.com/google/googletest

#define TEST_NAME typeid(*this).name()

namespace bdm {

// A function to test
int Compute42() { return 6 * 7; };

// Show how to compare two numbers
TEST(UtilTest, NumberTest) {
  // Expect equality
  EXPECT_EQ(Compute42(), 42);
}

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

}  // namespace bdm
