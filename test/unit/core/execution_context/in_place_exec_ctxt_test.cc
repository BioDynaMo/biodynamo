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

#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/grid.h"
#include "core/model_initializer.h"
#include "core/sim_object/cell.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(InPlaceExecutionContext, RemoveFromSimulation) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  Cell* cell_0 = new Cell();
  auto uid_0 = cell_0->GetUid();
  rm->push_back(cell_0);

  Cell* cell_1 = new Cell();
  auto uid_1 = cell_1->GetUid();
  rm->push_back(cell_1);

  Cell* cell_2 = new Cell();
  auto uid_2 = cell_2->GetUid();
  rm->push_back(cell_2);

  ctxt->RemoveFromSimulation(uid_0);
  ctxt->RemoveFromSimulation(uid_2);

  EXPECT_EQ(3u, rm->GetNumSimObjects());

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_1));
  EXPECT_FALSE(rm->Contains(uid_0));
  EXPECT_FALSE(rm->Contains(uid_2));
}

// Remove object that has been created in the same iteration. Thus it has not
// been added to the ResourceManager yet.
TEST(InPlaceExecutionContext, RemoveFromSimulationThatDoesNotExistInRm) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  Cell* cell_0 = new Cell();
  auto uid_0 = cell_0->GetUid();
  rm->push_back(cell_0);

  EXPECT_EQ(1u, rm->GetNumSimObjects());

  Cell* cell_1 = new Cell();
  auto uid_1 = cell_1->GetUid();
  ctxt->push_back(cell_1);

  EXPECT_EQ(1u, rm->GetNumSimObjects());

  ctxt->RemoveFromSimulation(uid_1);

  EXPECT_EQ(1u, rm->GetNumSimObjects());

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_FALSE(rm->Contains(uid_1));

  // check that the internal caches are properly cleared.
  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_FALSE(rm->Contains(uid_1));
}

TEST(InPlaceExecutionContext, NewAndGetSimObject) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  Cell* cell_0 = new Cell();
  cell_0->SetDiameter(123);
  auto uid_0 = cell_0->GetUid();
  rm->push_back(cell_0);

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_EQ(123, ctxt->GetSimObject(uid_0)->GetDiameter());
  EXPECT_EQ(123, rm->GetSimObject(uid_0)->GetDiameter());

  Cell* cell_1 = new Cell();
  auto uid_1 = cell_1->GetUid();
  cell_1->SetDiameter(456);
  ctxt->push_back(cell_1);

  EXPECT_EQ(1u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_FALSE(rm->Contains(uid_1));
  EXPECT_EQ(123, ctxt->GetSimObject(uid_0)->GetDiameter());
  EXPECT_EQ(123, rm->GetSimObject(uid_0)->GetDiameter());
  EXPECT_EQ(456, ctxt->GetSimObject(uid_1)->GetDiameter());

  ctxt->GetSimObject(uid_1)->SetDiameter(789);

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(2u, rm->GetNumSimObjects());
  EXPECT_TRUE(rm->Contains(uid_0));
  EXPECT_TRUE(rm->Contains(uid_1));
  EXPECT_EQ(123, ctxt->GetSimObject(uid_0)->GetDiameter());
  EXPECT_EQ(789, ctxt->GetSimObject(uid_1)->GetDiameter());
  EXPECT_EQ(123, rm->GetSimObject(uid_0)->GetDiameter());
  EXPECT_EQ(789, rm->GetSimObject(uid_1)->GetDiameter());
}

TEST(InPlaceExecutionContext, Execute) {
  Simulation sim(TEST_NAME);
  auto* ctxt = sim.GetExecutionContext();

  ctxt->DisableNeighborGuard();

  Cell cell_0;
  cell_0.SetDiameter(123);
  auto uid_0 = cell_0.GetUid();

  bool op1_called = false;
  bool op2_called = false;

  auto op1 = [&](SimObject* so) {
    // op1 must be  called first
    EXPECT_FALSE(op1_called);
    EXPECT_FALSE(op2_called);
    EXPECT_EQ(so->GetUid(), uid_0);
    op1_called = true;
  };

  auto op2 = [&](SimObject* so) {
    // op2 must be  called first
    EXPECT_TRUE(op1_called);
    EXPECT_FALSE(op2_called);
    EXPECT_EQ(so->GetUid(), uid_0);
    op2_called = true;
  };
  std::vector<std::function<void(SimObject*)>> operations = {op1, op2};
  ctxt->Execute(&cell_0, operations);

  EXPECT_TRUE(op1_called);
  EXPECT_TRUE(op2_called);
}

TEST(InPlaceExecutionContext, ExecuteThreadSafety) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();

  // create cells
  auto construct = [](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::Grid3D(32, 10, construct);

  // initialize
  const auto& all_exec_ctxts = sim.GetAllExecCtxts();
  all_exec_ctxts[0]->SetupIterationAll(all_exec_ctxts);
  sim.GetGrid()->Initialize();

  std::unordered_map<SoUid, uint64_t> num_neighbors;

  // this operation increases the diameter of the current sim_object and of all
  // its neighbors.
  auto op = [&](auto* so) {
    auto d = so->GetDiameter();
    so->SetDiameter(d + 1);

    uint64_t nb_counter = 0;
    auto nb_lambda = [&](const auto* neighbor) {
      auto* non_const_nb = rm->GetSimObject(neighbor->GetUid());
      auto d1 = non_const_nb->GetDiameter();
      non_const_nb->SetDiameter(d1 + 1);
      nb_counter++;
    };
    // ctxt must be obtained inside the lambda, otherwise we always get the
    // one corresponding to the master thread
    auto* ctxt = sim.GetExecutionContext();
    ctxt->ForEachNeighborWithinRadius(nb_lambda, *so, 100);
#pragma omp critical
    num_neighbors[so->GetUid()] = nb_counter;
  };

  rm->ApplyOnAllElementsParallel([&](SimObject* so) {
    // ctxt must be obtained inside the lambda, otherwise we always get the
    // one corresponding to the master thread
    auto* ctxt = sim.GetExecutionContext();
    ctxt->Execute(so, {op});
  });

  rm->ApplyOnAllElements([&](SimObject* so) {
    // expected diameter: initial value + num_neighbors + 1
    EXPECT_EQ(num_neighbors[so->GetUid()] + 11, so->GetDiameter());
  });
}

}  // namespace bdm
