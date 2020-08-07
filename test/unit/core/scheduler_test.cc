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
#include "core/operation/operation_registry.h"

namespace bdm {
namespace scheduler_test_internal {

#ifdef USE_DICT
TEST(SchedulerTest, NoRestoreFile) {
  auto set_param = [](auto* param) { param->restore_file_ = ""; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();

  remove(ROOTFILE);

  Cell* cell = new Cell();
  cell->SetDiameter(10);  // important for env to determine box size
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

  auto* env = simulation.GetEnvironment();
  std::array<int32_t, 2> expected_dim_threshold = {-10, 10};
  EXPECT_EQ(expected_dim_threshold, env->GetDimensionThresholds());
  std::array<int32_t, 6> expected_dimensions = {-10, 10, -10, 10, -10, 10};
  EXPECT_EQ(expected_dimensions, env->GetDimensions());
}

TEST(SchedulerTest, EmptySimulationAfterFirstIteration) {
  auto set_param = [](auto* param) {
    param->bound_space_ = true;
    param->min_bound_ = -10;
    param->max_bound_ = 10;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* env = simulation.GetEnvironment();
  auto* scheduler = simulation.GetScheduler();

  Cell* cell = new Cell(10);
  rm->push_back(cell);
  scheduler->Simulate(1);

  auto max_dimensions = env->GetDimensionThresholds();
  auto dimensions = env->GetDimensions();
  rm->Clear();

  scheduler->Simulate(1);

  EXPECT_EQ(max_dimensions, env->GetDimensionThresholds());
  EXPECT_EQ(dimensions, env->GetDimensions());
  EXPECT_FALSE(env->HasGrown());
}

struct TestOp : public OperationImpl {
  TestOp(uint64_t counter) : counter(counter) {}
  TestOp* Clone() override { return new TestOp(*this); }
  void operator()(SimObject* so) override { counter++; }
  uint64_t counter;

 private:
  static bool registered_;
};

REGISTER_OP(TestOp, "test_op", kCpu, 0)

TEST(SchedulerTest, OperationManagement) {
  Simulation simulation(TEST_NAME);

  simulation.GetResourceManager()->push_back(new Cell(10));

  auto* op1 = GET_OP("test_op");
  auto* op2 = GET_OP("test_op");
  
  auto* op1_impl = op1->GetImplementation<TestOp>();
  auto* op2_impl = op2->GetImplementation<TestOp>();
  
  // Change the state of one of the operations
  op1_impl->counter = 1;

  // schedule operations
  auto* scheduler = simulation.GetScheduler();
  scheduler->ScheduleOp(op1);
  scheduler->ScheduleOp(op2);
  scheduler->Simulate(10);
  EXPECT_EQ(11u, op1_impl->counter);
  EXPECT_EQ(10u, op2_impl->counter);

  // change frequency of operation
  op1->frequency_ = 3;
  scheduler->Simulate(10);
  EXPECT_EQ(14u, op1_impl->counter);
  EXPECT_EQ(20u, op2_impl->counter);

  // remove operation
  scheduler->UnscheduleOp(op2);
  scheduler->Simulate(10);
  EXPECT_EQ(17u, op1_impl->counter);
  EXPECT_EQ(20u, op2_impl->counter);

  // get non existing and protected operations
  scheduler->Simulate(10);
  EXPECT_EQ(21u, op1_impl->counter);
  EXPECT_EQ(20u, op2_impl->counter);
}

}  // namespace scheduler_test_internal
}  // namespace bdm
