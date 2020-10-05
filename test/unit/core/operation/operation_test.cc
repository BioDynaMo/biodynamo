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

#include "gtest/gtest.h"

#include "core/model_initializer.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/operation/reduction_op.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/sim_object/cell.h"
#include "core/simulation.h"

namespace bdm {

struct OperationTestOp : public StandaloneOperationImpl {
  void SetUp() override { setup_counter_++; }

  void operator()() override {}

  OperationTestOp* Clone() override { return new OperationTestOp(); }

  void TearDown() override { teardown_counter_++; }

  int setup_counter_ = 0;
  int teardown_counter_ = 0;

  static bool registered_;
};

BDM_REGISTER_OP(OperationTestOp, "OperationTestOp", kCpu);

TEST(OperationTest, SetupTearDown) {
  Simulation simulation("");
  simulation.GetResourceManager()->push_back(new Cell());
  auto* op = NewOperation("OperationTestOp");
  op->frequency_ = 2;
  simulation.GetScheduler()->ScheduleOp(op);

  simulation.Simulate(10);

  auto* op_impl = op->GetImplementation<OperationTestOp>();

  EXPECT_EQ(op_impl->setup_counter_, 5);
  EXPECT_EQ(op_impl->teardown_counter_, 5);
}

struct CheckDiameter : public Functor<void, SimObject*, int*> {
  explicit CheckDiameter(double d) : diameter_(d) {}

  void operator()(SimObject* so, int* tl_result) {
    if (so->GetDiameter() > diameter_) {
      (*tl_result)++;
    }
  }

  double diameter_;
};

struct CheckXPosition : public Functor<void, SimObject*, double*> {
  void operator()(SimObject* so, double* tl_result) {
    (*tl_result) += so->GetPosition()[0];
  }
};

/// 3x3x3 cells are positioned 50 units away from each other.
/// Each cell's diameter is equal to (1 + x_position / 10), so 1, 6 or 11.
///
///                                 +----------+
///                                 |          |
///                  +-----+        |          |
///                  |     |        |    11    |
///      +--+        |  6  |        |          |
///      |1 |        |     |        |          |
///      +--+        +-----+        +----------+
/// x->   0             50               100
///
/// We count the number of cells that have a diameter greater than 6
/// We count the average X position
TEST(OperationTest, ReductionOp) {
  // Lower the batch size such that multiple threads are working in parallel on
  // the operations (to test if multithreading doesn't cause race conditions)
  auto set_param = [](Param* param) {
    param->scheduling_batch_size_ = 3;
  };
  Simulation simulation("", set_param);
  auto* scheduler = simulation.GetScheduler();

  auto construct = [&](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(1 + position[0]/10);
    return cell;
  };
  ModelInitializer::Grid3D(3, 50, construct);

  // Count total number of simulation objects with a diameter greater than 6
  auto* op = NewOperation("ReductionOpInt");
  auto* op_impl = op->GetImplementation<ReductionOp<int>>();
  op_impl->Initialize(new CheckDiameter(6), new SumReduction<int>());
  scheduler->ScheduleOp(op);

  // Check average X position of all simulation objects
  auto* op_d = NewOperation("ReductionOpDouble");
  auto* op_d_impl = op_d->GetImplementation<ReductionOp<double>>();
  op_d_impl->Initialize(new CheckXPosition(), new SumReduction<double>());
  scheduler->ScheduleOp(op_d);

  simulation.Simulate(1);

  // Check the total number of sim objects with a diameter greater than 6
  EXPECT_EQ(9, op_impl->results_[0]);

  auto num_so = simulation.GetResourceManager()->GetNumSimObjects();

  // Check the average x position of all simulation objects
  EXPECT_EQ(50, op_d_impl->results_[0] / num_so);

  size_t num_threads = ThreadInfo::GetInstance()->GetMaxThreads();

  // Check if the arrays are of the right size (number of threads)
  EXPECT_EQ(num_threads, op_impl->tl_results_.size());
  EXPECT_EQ(num_threads, op_d_impl->tl_results_.size());
}

TEST(OperationTest, ReductionOpMultiThreading) {
  // Lower the batch size such that multiple threads are working in parallel on
  // the operations (to test if multithreading doesn't cause race conditions)
  auto set_param = [](Param* param) {
    param->scheduling_batch_size_ = 3;
  };
  Simulation simulation("", set_param);
  auto* scheduler = simulation.GetScheduler();

  auto construct = [&](const Double3& position) {
    Cell* cell = new Cell(position);
    return cell;
  };
  ModelInitializer::Grid3D(20, 3, construct);

  // Count total number of simulation objects with a diameter greater than 0
  auto* op = NewOperation("ReductionOpInt");
  auto* op_impl = op->GetImplementation<ReductionOp<int>>();
  op_impl->Initialize(new CheckDiameter(0), new SumReduction<int>());
  scheduler->ScheduleOp(op);

  simulation.Simulate(1);

  // Check the total number of sim objects with a diameter greater than 6
  EXPECT_EQ(8000, op_impl->results_[0]);
}

}  // namespace bdm
