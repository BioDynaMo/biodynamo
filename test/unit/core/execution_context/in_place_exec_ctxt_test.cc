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

#include "core/environment/environment.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/model_initializer.h"
#include "core/sim_object/cell.h"
#include "unit/test_util/test_sim_object.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace in_place_exec_ctxt_detail {

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

TEST(InPlaceExecutionContext, RemoveFromSimulationMultithreading) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  for (uint64_t i = 0; i < 1000; i++) {
    rm->push_back(new Cell());
  }

  EXPECT_EQ(1000u, rm->GetNumSimObjects());

#pragma omp parallel for
  for (uint64_t i = 0; i < 1000; i += 2) {
    sim.GetExecutionContext()->RemoveFromSimulation(SoUid(i));
  }

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(500u, rm->GetNumSimObjects());

  for (uint64_t i = 1; i < 100; i += 2) {
    EXPECT_TRUE(rm->Contains(SoUid(i)));
  }

  rm->ApplyOnAllElements(
      [](SimObject* so, SoHandle) { EXPECT_TRUE(so->GetUid() % 2 == 1); });
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

struct Op1 : public Operation {
  bool& op1_called;
  bool& op2_called;

  Op1(bool& op1_called, bool& op2_called)
      : Operation("op1"), op1_called(op1_called), op2_called(op2_called) {}

  void operator()(SimObject* so) override {
    // op1 must be  called first
    EXPECT_FALSE(op1_called);
    EXPECT_FALSE(op2_called);
    EXPECT_EQ(so->GetUid(), SoUid(0));
    op1_called = true;
  }
};

struct Op2 : public Operation {
  bool& op1_called;
  bool& op2_called;

  Op2(bool& op1_called, bool& op2_called)
      : Operation("op2"), op1_called(op1_called), op2_called(op2_called) {}

  void operator()(SimObject* so) override {
    // op2 must be  called first
    EXPECT_TRUE(op1_called);
    EXPECT_FALSE(op2_called);
    EXPECT_EQ(so->GetUid(), SoUid(0));
    op2_called = true;
  }
};

TEST(InPlaceExecutionContext, Execute) {
  Simulation sim(TEST_NAME);
  auto* ctxt = sim.GetExecutionContext();

  Cell cell_0;
  cell_0.SetDiameter(123);

  bool op1_called = false;
  bool op2_called = false;

  Op1 op1(op1_called, op2_called);
  Op2 op2(op1_called, op2_called);
  std::vector<Operation*> operations = {&op1, &op2};
  ctxt->Execute(&cell_0, operations);

  EXPECT_TRUE(op1_called);
  EXPECT_TRUE(op2_called);
}

struct NeighborFunctor : public Functor<void, const SimObject*, double> {
  NeighborFunctor(uint64_t& nb_counter) : nb_counter_(nb_counter) {}
  virtual ~NeighborFunctor() {}

  void operator()(const SimObject* neighbor, double squared_distance) override {
    auto* non_const_nb = const_cast<SimObject*>(neighbor);
    auto d1 = non_const_nb->GetDiameter();
    non_const_nb->SetDiameter(d1 + 1);
    nb_counter_++;
  }

 private:
  uint64_t& nb_counter_;
};

struct TestFunctor1 : public Functor<void, SimObject*> {
  Operation* op;

  TestFunctor1(Operation* op) : op(op) {}
  void operator()(SimObject* so) override {
    // ctxt must be obtained inside the lambda, otherwise we always get the
    // one corresponding to the master thread
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    ctxt->Execute(so, {op});
  }
};

struct TestOperation : public Operation {
  std::unordered_map<SoUid, uint64_t> num_neighbors;

  TestOperation() : Operation("op") {}
  void operator()(SimObject* so) override {
    auto d = so->GetDiameter();
    so->SetDiameter(d + 1);

    uint64_t nb_counter = 0;
    NeighborFunctor nb_functor(nb_counter);
    // ctxt must be obtained inside the lambda, otherwise we always get the
    // one corresponding to the master thread
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    ctxt->ForEachNeighborWithinRadius(nb_functor, *so, 101);
#pragma omp critical
    num_neighbors[so->GetUid()] = nb_counter;
  }
};

void RunInPlaceExecutionContextExecuteThreadSafety(
    Param::ThreadSafetyMechanism mechanism) {
  Simulation sim(
      "RunInPlaceExecutionContextExecuteThreadSafety",
      [&](Param* param) { param->thread_safety_mechanism_ = mechanism; });
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
  sim.GetEnvironment()->Update();

  // this operation increases the diameter of the current sim_object and of all
  // its neighbors.
  TestOperation op;

  TestFunctor1 functor1(&op);
  rm->ApplyOnAllElementsParallel(functor1);

  rm->ApplyOnAllElements([&](SimObject* so) {
    EXPECT_EQ(11 + op.num_neighbors[so->GetUid()], so->GetDiameter());
  });
}

TEST(InPlaceExecutionContext,
     ExecuteThreadSafetyTestWithUserSpecifiedThreadSafety) {
  RunInPlaceExecutionContextExecuteThreadSafety(
      Param::ThreadSafetyMechanism::kUserSpecified);
}

TEST(InPlaceExecutionContext, ExecuteThreadSafetyTestAutomaticThreadSafety) {
  RunInPlaceExecutionContextExecuteThreadSafety(
      Param::ThreadSafetyMechanism::kAutomatic);
}

TEST(InPlaceExecutionContext, PushBackMultithreadingTest) {
  Simulation simulation(TEST_NAME);

  std::vector<uint64_t> used_indexes;
  used_indexes.reserve(100000);

#pragma omp parallel for
  for (uint64_t i = 0; i < 100000; ++i) {
    auto* new_so = new TestSimObject();
    new_so->SetData(new_so->GetUid().GetIndex());

    auto* ctxt = simulation.GetExecutionContext();
    ctxt->push_back(new_so);

    auto* random = simulation.GetRandom();
    uint64_t random_number = 0;
    uint64_t read_index = 0;

// select random element between 0 and max uid index and check if the
// data of the simulation object is correct
#pragma omp critical
    {
      used_indexes.push_back(new_so->GetUid().GetIndex());
      random_number = static_cast<uint64_t>(
          std::round(random->Uniform(0, used_indexes.size() - 1)));
      read_index = used_indexes[random_number];
    }

    auto* tso =
        static_cast<TestSimObject*>(ctxt->GetSimObject(SoUid(read_index)));
    EXPECT_EQ(static_cast<uint64_t>(tso->GetData()), read_index);
  }
}

}  // namespace in_place_exec_ctxt_detail
}  // namespace bdm
