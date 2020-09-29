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

#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
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

}  // namespace bdm
