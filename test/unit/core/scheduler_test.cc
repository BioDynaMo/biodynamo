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

#ifdef USE_DICT
TEST(SchedulerTest, NoRestoreFile) {
  auto set_param = [](auto* param) { param->restore_file_ = ""; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  remove(ROOTFILE);

  Cell* cell = new Cell();
  cell->SetDiameter(10);  // important for grid to determine box size
  rm->push_back(cell);

  // start restore validation
  TestSchedulerRestore scheduler;
  scheduler.Simulate(100);
  EXPECT_EQ(100u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->GetNumSimObjects());

  scheduler.Simulate(100);
  EXPECT_EQ(200u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->GetNumSimObjects());

  scheduler.Simulate(100);
  EXPECT_EQ(300u, scheduler.execute_calls);
  EXPECT_EQ(1u, rm->GetNumSimObjects());
}

TEST(SchedulerTest, Restore) { RunRestoreTest(); }

TEST(SchedulerTest, Backup) { RunBackupTest(); }
#endif  // USE_DICT

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

  Cell* cell = new Cell(10);
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

struct TestOp : public Operation {
  TestOp(const std::string& id) : Operation(id) {}
  void operator()(SimObject* so) override { counter++; }
  uint64_t counter=0;
};

TEST(SchedulerTest, OperationManagement) {
  Simulation simulation(TEST_NAME);

  simulation.GetResourceManager()->push_back(new Cell(10));

  auto* op1 = new TestOp("op1");
  auto* op2 = new TestOp("op2");

  // add operations
  auto* scheduler = simulation.GetScheduler();
  scheduler->AddOperation(op1);
  scheduler->AddOperation(op2);
  scheduler->Simulate(10);
  EXPECT_EQ(10u, op1->counter);
  EXPECT_EQ(10u, op2->counter);

  // change frequency of operation
  scheduler->GetOperation(op1->name_)->frequency_ = 3;
  scheduler->Simulate(10);
  EXPECT_EQ(13u, op1->counter);
  EXPECT_EQ(20u, op2->counter);

  // remove operation
  scheduler->RemoveOperation(op2->name_);
  scheduler->Simulate(10);
  EXPECT_EQ(16u, op1->counter);
  EXPECT_EQ(20u, op2->counter);

  // get non existing and protected operations
  EXPECT_TRUE(scheduler->GetOperation("does not exist") == nullptr);
  EXPECT_TRUE(scheduler->GetOperation("first") == nullptr);  // is protected
  scheduler->Simulate(10);
  EXPECT_EQ(20u, op1->counter);
  EXPECT_EQ(20u, op2->counter);
}

}  // namespace scheduler_test_internal
}  // namespace bdm
