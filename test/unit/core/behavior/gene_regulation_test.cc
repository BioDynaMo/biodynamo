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

#ifndef UNIT_BEHAVIOR_GENE_REGULATION_TEST
#define UNIT_BEHAVIOR_GENE_REGULATION_TEST

#include "core/behavior/gene_regulation.h"
#include "core/agent/cell.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace gene_regulation_test_internal {

struct TestScheduler : public Scheduler {
  void SetSimulationSteps(uint64_t total_steps) { total_steps_ = total_steps; }
};

TEST(GeneRegulationTest, EulerTest) {
  auto set_param = [](auto* param) {
    param->numerical_ode_solver = Param::NumericalODESolver::kEuler;
    param->min_bound = 0;
    param->max_bound = 100;
    param->simulation_time_step = 0.01;
  };
  Simulation simulation(TEST_NAME, set_param);
  auto* scheduler = new TestScheduler();
  simulation.ReplaceScheduler(scheduler);

  scheduler->SetSimulationSteps(1);

  auto func1 = [](real_t curr_time, real_t last_concentration) {
    return curr_time * last_concentration;
  };
  auto func2 = [](real_t curr_time, real_t last_concentration) {
    return curr_time * last_concentration + 1;
  };
  auto func3 = [](real_t curr_time, real_t last_concentration) {
    return curr_time * last_concentration + 2;
  };

  GeneRegulation gene_regulation;
  gene_regulation.AddGene(func1, 3);
  gene_regulation.AddGene(func2, 3);
  gene_regulation.AddGene(func3, 3);
  Cell cell;
  gene_regulation.Run(&cell);

  const auto& concentrations = gene_regulation.GetValues();
  EXPECT_NEAR(real_t(3.0003000000000002), concentrations[0],
              abs_error<real_t>::value);
  EXPECT_NEAR(real_t(3.0103), concentrations[1], abs_error<real_t>::value);
  EXPECT_NEAR(real_t(3.0203000000000002), concentrations[2],
              abs_error<real_t>::value);
}

// Example 1 from:
// https://ece.uwaterloo.ca/~dwharder/NumericalAnalysis/14IVPs/rk/examples.html
TEST(GeneRegulationTest, RK4Test) {
  auto set_param = [](auto* param) {
    param->numerical_ode_solver = Param::NumericalODESolver::kRK4;
    param->simulation_time_step = 1;
  };
  Simulation simulation(TEST_NAME, set_param);

  GeneRegulation gene_regulation;
  gene_regulation.AddGene(
      [](real_t curr_time, real_t last_concentration) {
        return 1 - curr_time * last_concentration;
      },
      1);
  Cell cell;
  gene_regulation.Run(&cell);

  const auto& concentrations = gene_regulation.GetValues();
  EXPECT_REAL_EQ(real_t(1.3229166666666665), concentrations[0]);
}

}  // namespace gene_regulation_test_internal
}  // namespace bdm

#endif  // UNIT_BEHAVIOR_GENE_REGULATION_TEST
