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

#include "unit/scheduler_test.h"

namespace bdm {
namespace scheduler_test_internal {

TEST(SchedulerTest, NoRestoreFile) {
  auto set_param = [](auto* param) { param->restore_file_ = ""; };
  Simulation<> simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  remove(ROOTFILE);

  Cell cell;
  cell.SetDiameter(10);  // important for grid to determine box size
  rm->Get<Cell>()->push_back(cell);

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

}  // namespace scheduler_test_internal
}  // namespace bdm
