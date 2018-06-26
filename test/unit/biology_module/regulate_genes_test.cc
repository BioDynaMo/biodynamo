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

#ifndef UNIT_BIOLOGY_MODULE_REGULATE_GENES_TEST
#define UNIT_BIOLOGY_MODULE_REGULATE_GENES_TEST

#include "biology_module/regulate_genes.h"
#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"

namespace bdm {
namespace regulate_genes_test_internal {

struct TestCell {};
struct TestScheduler : public Scheduler<> {
  void SetSimulationSteps(uint64_t total_steps) { total_steps_ = total_steps; }
};

TEST(RegulateGenesTest, EulerTest) {
  Simulation<> simulation(TEST_NAME);
  auto* param = simulation.GetParam();
  auto* scheduler = new TestScheduler();
  simulation.ReplaceScheduler(scheduler);

  param->numerical_ode_solver_ = param->NumericalODESolver::kEuler;
  scheduler->SetSimulationSteps(1);

  auto func1 = [](double curr_time, double last_concentration) {
    return curr_time * last_concentration;
  };
  auto func2 = [](double curr_time, double last_concentration) {
    return curr_time * last_concentration + 1;
  };
  auto func3 = [](double curr_time, double last_concentration) {
    return curr_time * last_concentration + 2;
  };

  RegulateGenes regulate_genes;
  regulate_genes.AddGene(func1, 3);
  regulate_genes.AddGene(func2, 3);
  regulate_genes.AddGene(func3, 3);
  TestCell cell;
  regulate_genes.Run(&cell);

  const auto& concentrations = regulate_genes.GetConcentrations();
  EXPECT_NEAR(3.0003000000000002, concentrations[0], 1e-9);
  EXPECT_NEAR(3.0103, concentrations[1], 1e-9);
  EXPECT_NEAR(3.0203000000000002, concentrations[2], 1e-9);
}

// Example 1 from:
// https://ece.uwaterloo.ca/~dwharder/NumericalAnalysis/14IVPs/rk/examples.html
TEST(RegulateGenesTest, RK4Test) {
  Simulation<> simulation(TEST_NAME);
  auto* param = simulation.GetParam();

  param->numerical_ode_solver_ = param->NumericalODESolver::kRK4;
  // simulation steps is zero by default
  param->simulation_time_step_ = 1;

  RegulateGenes regulate_genes;
  regulate_genes.AddGene(
      [](double curr_time, double last_concentration) {
        return 1 - curr_time * last_concentration;
      },
      1);
  TestCell cell;
  regulate_genes.Run(&cell);

  const auto& concentrations = regulate_genes.GetConcentrations();
  EXPECT_NEAR(1.3229166667, concentrations[0], 1e-9);
}

}  // namespace regulate_genes_test_internal
}  // namespace bdm

#endif  // UNIT_BIOLOGY_MODULE_REGULATE_GENES_TEST
