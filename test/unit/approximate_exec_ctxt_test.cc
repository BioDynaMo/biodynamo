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

#include <gtest/gtest.h>

#include "cell.h"
#include "default_ctparam.h"
#include "execution_context/approximate_exec_ctxt.h"
#include "simulation_implementation.h"
#include "test_util.h"

namespace bdm {


TEST(ApproximateExecCtxt, RemoveFromSimulation) {
  Simulation<> sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecCtxt();

  Cell cell_0;
  auto uid_0 = cell_0.GetUid();
  rm->push_back(cell_0);

  Cell cell_1;
  auto uid_1 = cell_1.GetUid();
  rm->push_back(cell_1);

  Cell cell_2;
  auto uid_2 = cell_2.GetUid();
  rm->push_back(cell_2);

  ctxt->RemoveFromSimulation(uid_0);
  ctxt->RemoveFromSimulation(uid_2);

  EXPECT_EQ(3u, rm->GetNumSimObjects());

  ctxt->TearDownIteration();

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_1));
  EXPECT_FALSE(rm->Contains(uid_0));
  EXPECT_FALSE(rm->Contains(uid_2));
}

// Remove object that has been created in the same iteration. Thus it has not
// been added to the ResourceManager yet.
TEST(ApproximateExecCtxt, RemoveFromSimulationThatDoesNotExistInRm) {
  Simulation<> sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecCtxt();

  Cell cell_0;
  auto uid_0 = cell_0.GetUid();
  rm->push_back(cell_0);

  EXPECT_EQ(1u, rm->GetNumSimObjects());

  auto&& cell_1 = ctxt->New<Cell>();
  auto uid_1 = cell_1->GetUid();

  EXPECT_EQ(1u, rm->GetNumSimObjects());

  ctxt->RemoveFromSimulation(uid_1);

  EXPECT_EQ(1u, rm->GetNumSimObjects());

  ctxt->TearDownIteration();

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_FALSE(rm->Contains(uid_1));
}

TEST(ApproximateExecCtxt, NewAndGetSimObject) {
  Simulation<> sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecCtxt();

  Cell cell_0;
  cell_0.SetDiameter(123);
  auto uid_0 = cell_0.GetUid();
  rm->push_back(cell_0);

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_EQ(123, ctxt->GetSimObject<Cell>(uid_0).GetDiameter());
  EXPECT_EQ(123, rm->GetSimObject<Cell>(uid_0).GetDiameter());

  auto&& cell_1 = ctxt->New<Cell>();
  cell_1->SetDiameter(456);
  auto uid_1 = cell_1->GetUid();

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_FALSE(rm->Contains(uid_1));
  EXPECT_EQ(123, ctxt->GetSimObject<Cell>(uid_0).GetDiameter());
  EXPECT_EQ(123, rm->GetSimObject<Cell>(uid_0).GetDiameter());
  EXPECT_EQ(456, ctxt->GetSimObject<Cell>(uid_1).GetDiameter());

  ctxt->GetSimObject<Cell>(uid_1).SetDiameter(789);

  ctxt->TearDownIteration();

  EXPECT_EQ(2u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_TRUE(rm->Contains(uid_1));
  EXPECT_EQ(123, ctxt->GetSimObject<Cell>(uid_0).GetDiameter());
  EXPECT_EQ(789, ctxt->GetSimObject<Cell>(uid_1).GetDiameter());
  EXPECT_EQ(123, rm->GetSimObject<Cell>(uid_0).GetDiameter());
  EXPECT_EQ(789, rm->GetSimObject<Cell>(uid_1).GetDiameter());
}

TEST(ApproximateExecCtxt, Execute) {
  Simulation<> sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecCtxt();

  Cell cell_0;
  cell_0.SetDiameter(123);
  auto uid_0 = cell_0.GetUid();

  bool op1_called = false;
  bool op2_called = false;

  auto op1 = [&](auto&& so) {
    // op1 must be  called first
    EXPECT_FALSE(op1_called);
    EXPECT_FALSE(op2_called);
    EXPECT_EQ(so.GetUid(), uid_0);
    op1_called = true;
  };

  auto op2 = [&](auto&& so) {
    // op2 must be  called first
    EXPECT_TRUE(op1_called);
    EXPECT_FALSE(op2_called);
    EXPECT_EQ(so.GetUid(), uid_0);
    op2_called = true;
  };

  ctxt->Execute(cell_0, op1, op2);

  EXPECT_TRUE(op1_called);
  EXPECT_TRUE(op2_called);
}

}  // namespace bdm
