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

#include "unit/core/scheduler_test.h"

namespace bdm {
namespace scheduler_test_internal {

TEST(SchedulerTest, NoRestoreFile) {
  auto set_param = [](auto* param) { param->restore_file_ = ""; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  remove(ROOTFILE);

  Cell cell;
  cell.SetDiameter(10);  // important for grid to determine box size
  rm->push_back(cell);

  // start restore validation
  TestSchedulerRestore scheduler;
  scheduler.Simulate(100);
  EXPECT_EQ(100u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->Get<Cell>()->size());

  scheduler.Simulate(100);
  EXPECT_EQ(200u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->Get<Cell>()->size());

  scheduler.Simulate(100);
  EXPECT_EQ(300u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->Get<Cell>()->size());
}

TEST(SchedulerTest, Restore) { RunRestoreTest(); }

TEST(SchedulerTest, Backup) { RunBackupTest(); }

TEST(SchedulerTest, EmptySimulationFromBeginning) {
  auto set_param = [](auto* param) {
    param->bound_space_ = true;
    param->min_bound_ = -10;
    param->max_bound_ = 10;
  };
  Simulation simulation(TEST_NAME, set_param);

  simulation.GetScheduler()->Simulate(1);

  auto* grid = simulation.GetGrid();
  std::array<int32_t, 2> expected_dim_threshold = {-10, 10};
  EXPECT_EQ(expected_dim_threshold, grid->GetDimensionThresholds());
  std::array<int32_t, 6> expected_dimensions = {-10, 10, -10, 10, -10, 10};
  EXPECT_EQ(expected_dimensions, grid->GetDimensions());
}

TEST(SchedulerTest, EmptySimulationAfterFirstIteration) {
  auto set_param = [](auto* param) {
    param->bound_space_ = true;
    param->min_bound_ = -10;
    param->max_bound_ = 10;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();
  auto* scheduler = simulation.GetScheduler();

  Cell cell(10);
  rm->push_back(cell);
  scheduler->Simulate(1);

  auto max_dimensions = grid->GetDimensionThresholds();
  auto dimensions = grid->GetDimensions();
  rm->Clear();

  scheduler->Simulate(1);

  EXPECT_EQ(max_dimensions, grid->GetDimensionThresholds());
  EXPECT_EQ(dimensions, grid->GetDimensions());
  EXPECT_FALSE(grid->HasGrown());
}

}  // namespace scheduler_test_internal
}  // namespace bdm
