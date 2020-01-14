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

#ifndef UNIT_CORE_SCHEDULER_TEST_H_
#define UNIT_CORE_SCHEDULER_TEST_H_

#include "core/scheduler.h"

#include <gtest/gtest.h>
#include <unistd.h>
#include <string>

#include "core/grid.h"
#include "core/sim_object/cell.h"
#include "core/simulation_backup.h"
#include "core/util/io.h"
#include "unit/test_util/test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace scheduler_test_internal {

class TestSchedulerRestore : public Scheduler {
 public:
  void Execute() override { execute_calls++; }

  unsigned execute_calls = 0;
};

class TestSchedulerBackup : public Scheduler {
 public:
  void Execute() override {
    // sleep
    usleep(350000);
    // backup should be created every second -> every three iterations
    if (execute_calls_ % 3 != 0 || execute_calls_ == 0) {
      EXPECT_FALSE(FileExists(ROOTFILE));
    } else {
      EXPECT_TRUE(FileExists(ROOTFILE));
      remove(ROOTFILE);
    }
    execute_calls_++;
  }
  unsigned execute_calls_ = 0;
};

inline void RunRestoreTest() {
  {
    Simulation simulation("SchedulerTest_RunRestoreTest");
    auto* rm = simulation.GetResourceManager();
    remove(ROOTFILE);

    // create backup that will be restored later on
    Cell* cell = new Cell();
    cell->SetDiameter(10);  // important for grid to determine box size
    rm->push_back(cell);
    SimulationBackup backup(ROOTFILE, "");
    backup.Backup(149);
    rm->Clear();
    EXPECT_EQ(0u, rm->GetNumSimObjects());
  }

  // start restore validation
  auto set_param = [](auto* param) { param->restore_file_ = ROOTFILE; };
  Simulation simulation("SchedulerTest_RunRestoreTest", set_param);
  auto* rm = simulation.GetResourceManager();
  TestSchedulerRestore scheduler;
  // 149 simulation steps have already been calculated. Therefore, this call
  // should be ignored
  scheduler.Simulate(100);
  EXPECT_EQ(0u, scheduler.execute_calls);
  EXPECT_EQ(0u, rm->GetNumSimObjects());

  // Restore should happen within this call
  scheduler.Simulate(100);
  //   only 51 steps should be simulated
  EXPECT_EQ(51u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->GetNumSimObjects());

  // add element to see if if restore happens again
  rm->push_back(new Cell());

  // normal simulation - no restore
  scheduler.Simulate(100);
  EXPECT_EQ(151u, scheduler.execute_calls);
  EXPECT_EQ(2u, rm->GetNumSimObjects());

  remove(ROOTFILE);
}

inline void RunBackupTest() {
  auto set_param = [](auto* param) {
    param->backup_file_ = ROOTFILE;
    param->backup_interval_ = 1;
  };

  Simulation simulation("SchedulerTest_RunBackupTest", set_param);
  auto* rm = simulation.GetResourceManager();

  remove(ROOTFILE);

  Cell* cell = new Cell();
  cell->SetDiameter(10);  // important for grid to determine box size
  rm->push_back(cell);

  TestSchedulerBackup scheduler;

  // one simulation step takes 350 ms -> backup should be created every three
  // steps
  scheduler.Simulate(4);
  remove(ROOTFILE);
}

}  // namespace scheduler_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_SCHEDULER_TEST_H_
