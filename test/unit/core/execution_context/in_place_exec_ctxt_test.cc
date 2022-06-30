// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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

#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/model_initializer.h"
#include "core/operation/operation_registry.h"
#include "core/randomized_rm.h"  // for bdm::Ubrng
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace in_place_exec_ctxt_detail {

TEST(InPlaceExecutionContext, RemoveAgent) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  Cell* cell_0 = new Cell();
  auto uid_0 = cell_0->GetUid();
  rm->AddAgent(cell_0);

  Cell* cell_1 = new Cell();
  auto uid_1 = cell_1->GetUid();
  rm->AddAgent(cell_1);

  Cell* cell_2 = new Cell();
  auto uid_2 = cell_2->GetUid();
  rm->AddAgent(cell_2);

  ctxt->RemoveAgent(uid_0);
  ctxt->RemoveAgent(uid_2);

  EXPECT_EQ(3u, rm->GetNumAgents());

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(1u, rm->GetNumAgents());
  EXPECT_TRUE(rm->ContainsAgent(uid_1));
  EXPECT_FALSE(rm->ContainsAgent(uid_0));
  EXPECT_FALSE(rm->ContainsAgent(uid_2));
}

void RunRemoveAgentMultithreadingTest(const char* name, uint64_t num_removed) {
  Simulation sim(name);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  // Create agents in all numa domains
#pragma omp parallel for
  for (uint64_t i = 0; i < 1000; i++) {
    sim.GetExecutionContext()->AddAgent(new Cell());
  }

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());
  EXPECT_EQ(1000u, rm->GetNumAgents());

  // created shuffled list of uids
  std::vector<AgentUid> uids(1000);
  for (uint64_t i = 0; i < 1000; i++) {
    uids[i] = AgentUid(i);
  }
  auto* random = sim.GetRandom();
  std::shuffle(uids.begin(), uids.end(), Ubrng(random));

#pragma omp parallel for
  for (uint64_t i = 0; i < num_removed; ++i) {
    sim.GetExecutionContext()->RemoveAgent(uids[i]);
  }

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(1000u - num_removed, rm->GetNumAgents());

  // Removed agents should be removed from ResourceManager
  for (uint64_t i = 0; i < num_removed; i += 2) {
    EXPECT_FALSE(rm->ContainsAgent(uids[i]));
  }
  // Remaining ones should be still there
  for (uint64_t i = num_removed; i < 1000u; i += 2) {
    EXPECT_TRUE(rm->ContainsAgent(uids[i]));
  }
}

TEST(InPlaceExecutionContext, RemoveAgentMultithreading) {
  RunRemoveAgentMultithreadingTest(TEST_NAME, 0);
  RunRemoveAgentMultithreadingTest(TEST_NAME, 1);
  auto* tinfo = ThreadInfo::GetInstance();
  RunRemoveAgentMultithreadingTest(TEST_NAME, tinfo->GetMaxThreads());
  RunRemoveAgentMultithreadingTest(TEST_NAME, 100);
  RunRemoveAgentMultithreadingTest(TEST_NAME, 250);
  RunRemoveAgentMultithreadingTest(TEST_NAME, 500);
  RunRemoveAgentMultithreadingTest(TEST_NAME, 750);
  RunRemoveAgentMultithreadingTest(TEST_NAME, 998);
  RunRemoveAgentMultithreadingTest(TEST_NAME, 999);
  RunRemoveAgentMultithreadingTest(TEST_NAME, 1000);
}

// Remove object that has been created in the same iteration. Thus it has not
// been added to the ResourceManager yet.
TEST(InPlaceExecutionContext, RemoveAgentThatDoesNotExistInRm) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  Cell* cell_0 = new Cell();
  auto uid_0 = cell_0->GetUid();
  rm->AddAgent(cell_0);

  EXPECT_EQ(1u, rm->GetNumAgents());

  Cell* cell_1 = new Cell();
  auto uid_1 = cell_1->GetUid();
  ctxt->AddAgent(cell_1);

  EXPECT_EQ(1u, rm->GetNumAgents());

  ctxt->RemoveAgent(uid_1);

  EXPECT_EQ(1u, rm->GetNumAgents());

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(1u, rm->GetNumAgents());
  EXPECT_TRUE(rm->ContainsAgent(uid_0));
  EXPECT_FALSE(rm->ContainsAgent(uid_1));

  // check that the internal caches are properly cleared.
  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(1u, rm->GetNumAgents());
  EXPECT_TRUE(rm->ContainsAgent(uid_0));
  EXPECT_FALSE(rm->ContainsAgent(uid_1));
}

TEST(InPlaceExecutionContext, NewAndGetAgent) {
  Simulation sim(TEST_NAME);
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  Cell* cell_0 = new Cell();
  cell_0->SetDiameter(123);
  auto uid_0 = cell_0->GetUid();
  rm->AddAgent(cell_0);

  EXPECT_EQ(1u, rm->GetNumAgents());
  EXPECT_TRUE(rm->ContainsAgent(uid_0));
  EXPECT_EQ(123, ctxt->GetAgent(uid_0)->GetDiameter());
  EXPECT_EQ(123, rm->GetAgent(uid_0)->GetDiameter());

  Cell* cell_1 = new Cell();
  auto uid_1 = cell_1->GetUid();
  cell_1->SetDiameter(456);
  ctxt->AddAgent(cell_1);

  EXPECT_EQ(1u, rm->GetNumAgents());
  EXPECT_TRUE(rm->ContainsAgent(uid_0));
  EXPECT_FALSE(rm->ContainsAgent(uid_1));
  EXPECT_EQ(123, ctxt->GetAgent(uid_0)->GetDiameter());
  EXPECT_EQ(123, rm->GetAgent(uid_0)->GetDiameter());
  EXPECT_EQ(456, ctxt->GetAgent(uid_1)->GetDiameter());

  ctxt->GetAgent(uid_1)->SetDiameter(789);

  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());

  EXPECT_EQ(2u, rm->GetNumAgents());
  EXPECT_TRUE(rm->ContainsAgent(uid_0));
  EXPECT_TRUE(rm->ContainsAgent(uid_1));
  EXPECT_EQ(123, ctxt->GetAgent(uid_0)->GetDiameter());
  EXPECT_EQ(789, ctxt->GetAgent(uid_1)->GetDiameter());
  EXPECT_EQ(123, rm->GetAgent(uid_0)->GetDiameter());
  EXPECT_EQ(789, rm->GetAgent(uid_1)->GetDiameter());
}

struct Op1 : public AgentOperationImpl {
  BDM_OP_HEADER(Op1);

  bool* op1_called_;
  bool* op2_called_;

  void operator()(Agent* agent) override {
    // op1 must be  called first
    EXPECT_FALSE(*op1_called_);
    EXPECT_FALSE(*op2_called_);
    EXPECT_EQ(agent->GetUid(), AgentUid(0));
    *op1_called_ = true;
  }
};

BDM_REGISTER_OP(Op1, "Op1", kCpu);

struct Op2 : public AgentOperationImpl {
  BDM_OP_HEADER(Op2);

  bool* op1_called_;
  bool* op2_called_;

  void operator()(Agent* agent) override {
    // op2 must be  called first
    EXPECT_TRUE(*op1_called_);
    EXPECT_FALSE(*op2_called_);
    EXPECT_EQ(agent->GetUid(), AgentUid(0));
    *op2_called_ = true;
  }
};

BDM_REGISTER_OP(Op2, "Op2", kCpu);

TEST(InPlaceExecutionContext, Execute) {
  Simulation sim(TEST_NAME);
  auto* ctxt = sim.GetExecutionContext();

  Cell cell_0;
  cell_0.SetDiameter(123);

  bool op1_called = false;
  bool op2_called = false;

  auto* op1 = NewOperation("Op1");
  auto* op2 = NewOperation("Op2");
  op1->GetImplementation<Op1>()->op1_called_ = &op1_called;
  op1->GetImplementation<Op1>()->op2_called_ = &op2_called;
  op2->GetImplementation<Op2>()->op1_called_ = &op1_called;
  op2->GetImplementation<Op2>()->op2_called_ = &op2_called;
  std::vector<Operation*> operations = {op1, op2};
  ctxt->Execute(&cell_0, AgentHandle(0, 0), operations);

  EXPECT_TRUE(op1_called);
  EXPECT_TRUE(op2_called);

  delete op1;
  delete op2;
}

struct NeighborFunctor : public Functor<void, Agent*, real_t> {
  NeighborFunctor(uint64_t& nb_counter) : nb_counter_(nb_counter) {}
  virtual ~NeighborFunctor() = default;

  void operator()(Agent* neighbor, real_t squared_distance) override {
    auto* non_const_nb = const_cast<Agent*>(neighbor);
    auto d1 = non_const_nb->GetDiameter();
    non_const_nb->SetDiameter(d1 + 1);
    nb_counter_++;
  }

 private:
  uint64_t& nb_counter_;
};

struct TestFunctor1 : public Functor<void, Agent*> {
  Operation* op;

  TestFunctor1(Operation* op) : op(op) {}
  void operator()(Agent* agent) override {
    // ctxt must be obtained inside the lambda, otherwise we always get the
    // one corresponding to the master thread
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    ctxt->Execute(agent, AgentHandle(0, 0), {op});
  }
};

struct TestFunctor2 : public Functor<void, Agent*> {
  Operation* op;

  TestFunctor2(Operation* op) : op(op) {}
  void operator()(Agent* agent) override {
    // ctxt must be obtained inside the lambda, otherwise we always get the
    // one corresponding to the master thread
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    ctxt->Execute(agent, AgentHandle(0, 0), {op});
  }
};

struct TestOperation : public AgentOperationImpl {
  BDM_OP_HEADER(TestOperation);

  std::unordered_map<AgentUid, uint64_t> num_neighbors;

  void operator()(Agent* agent) override {
    auto d = agent->GetDiameter();
    agent->SetDiameter(d + 1);

    uint64_t nb_counter = 0;
    NeighborFunctor nb_functor(nb_counter);
    // ctxt must be obtained inside the lambda, otherwise we always get the
    // one corresponding to the master thread
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    auto* env = Simulation::GetActive()->GetEnvironment();
    ctxt->ForEachNeighbor(nb_functor, *agent,
                          env->GetLargestAgentSizeSquared());
#pragma omp critical
    num_neighbors[agent->GetUid()] = nb_counter;
  }
};

BDM_REGISTER_OP(TestOperation, "TestOperation", kCpu);

void RunInPlaceExecutionContextExecuteThreadSafety(
    Param::ThreadSafetyMechanism mechanism) {
  Simulation sim(
      "RunInPlaceExecutionContextExecuteThreadSafety",
      [&](Param* param) { param->thread_safety_mechanism = mechanism; });
  auto* rm = sim.GetResourceManager();

  // create cells
  auto construct = [](const Real3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::Grid3D(32, 10, construct);

  // initialize
  const auto& all_exec_ctxts = sim.GetAllExecCtxts();
  all_exec_ctxts[0]->SetupIterationAll(all_exec_ctxts);
  sim.GetEnvironment()->Update();

  // this operation increases the diameter of the current agent and of all
  // its neighbors.
  auto* op1 = NewOperation("TestOperation");
  TestFunctor1 functor1(op1);
  rm->ForEachAgentParallel(functor1);

  auto* op2 = NewOperation("TestOperation");
  TestFunctor2 functor2(op2);
  rm->ForEachAgent(functor2);

  delete op1;
  delete op2;
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
    auto* new_agent = new TestAgent();
    new_agent->SetData(new_agent->GetUid().GetIndex());

    auto* ctxt = simulation.GetExecutionContext();
    ctxt->AddAgent(new_agent);

    auto* random = simulation.GetRandom();
    uint64_t random_number = 0;
    uint64_t read_index = 0;

// select random element between 0 and max uid index and check if the
// data of the agent is correct
#pragma omp critical
    {
      used_indexes.push_back(new_agent->GetUid().GetIndex());
      random_number = static_cast<uint64_t>(
          std::round(random->Uniform(0, used_indexes.size() - 1)));
      read_index = used_indexes[random_number];
    }

    auto* tagent =
        static_cast<TestAgent*>(ctxt->GetAgent(AgentUid(read_index)));
    EXPECT_EQ(static_cast<uint64_t>(tagent->GetData()), read_index);
  }
}

TEST(InPlaceExecutionContext, DefaultSearchRadius) {
  Simulation sim(TEST_NAME);
  auto* env = sim.GetEnvironment();
  auto* rm = sim.GetResourceManager();
  auto* ctxt = sim.GetExecutionContext();

  Cell* cell_0 = new Cell();
  cell_0->SetDiameter(42);
  rm->AddAgent(cell_0);

  EXPECT_EQ(1u, rm->GetNumAgents());
  EXPECT_EQ(env->GetLargestAgentSizeSquared(), 0.0);

  env->Update();
  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());
  EXPECT_EQ(42 * 42, env->GetLargestAgentSizeSquared());

  // Add agent with new largest object size
  Cell* cell_1 = new Cell();
  cell_1->SetDiameter(43);
  rm->AddAgent(cell_1);

  env->Update();
  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());
  EXPECT_EQ(43 * 43, env->GetLargestAgentSizeSquared());
}

struct TestNeighborFunctor : public Functor<void, Agent*, real_t> {
  void operator()(Agent* neighbor, real_t squared_distance) override {}
};

TEST(InPlaceExecutionContext, NeighborCacheValidity) {
  auto set_param = [](Param* param) { param->cache_neighbors = true; };
  Simulation sim(TEST_NAME, set_param);
  auto* env = sim.GetEnvironment();
  auto* rm = sim.GetResourceManager();
  auto* ctxt =
      dynamic_cast<InPlaceExecutionContext*>(sim.GetExecutionContext());

  for (int i = 0; i < 10; i++) {
    Cell* cell = new Cell();
    cell->SetDiameter(5);
    rm->AddAgent(cell);
  }

  EXPECT_EQ(10u, rm->GetNumAgents());
  EXPECT_EQ(env->GetLargestAgentSizeSquared(), 0.0);

  env->Update();
  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());
  EXPECT_EQ(5 * 5, env->GetLargestAgentSizeSquared());

  Cell* cell_1 = new Cell();
  cell_1->SetDiameter(6);
  rm->AddAgent(cell_1);

  env->Update();
  ctxt->TearDownIterationAll(sim.GetAllExecCtxts());
  EXPECT_EQ(6 * 6, env->GetLargestAgentSizeSquared());
  EXPECT_TRUE(ctxt->cache_neighbors_);
  EXPECT_FALSE(ctxt->IsNeighborCacheValid(env->GetLargestAgentSizeSquared()));

  // Since we didn't run a ForEachNeighbor operation, the cached squared radius
  // is still its default value of 0.0
  EXPECT_FALSE(ctxt->IsNeighborCacheValid(4 * 4));

  TestNeighborFunctor test_functor;
  ctxt->ForEachNeighbor(test_functor, *cell_1, 3 * 3);
  EXPECT_EQ(3 * 3, ctxt->cached_squared_search_radius_);
  EXPECT_TRUE(ctxt->IsNeighborCacheValid(3 * 3));
  EXPECT_FALSE(ctxt->IsNeighborCacheValid(4 * 4));

  // Request larger neighborhood and invalidate existing cache
  ctxt->ForEachNeighbor(test_functor, *cell_1, 4 * 4);
  EXPECT_EQ(4 * 4, ctxt->cached_squared_search_radius_);
  EXPECT_TRUE(ctxt->IsNeighborCacheValid(4 * 4 - 1));
  EXPECT_FALSE(ctxt->IsNeighborCacheValid(4 * 4 + 1));
}

TEST(InPlaceExecutionContext, ForEachNeighbor) {
  Simulation sim("ForEachNeighbor",
                 [&](Param* param) { param->cache_neighbors = true; });
  auto* rm = sim.GetResourceManager();

  // create cells
  auto construct = [](const Real3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(20);
    return cell;
  };
  ModelInitializer::Grid3D(3, 10, construct);

  // initialize
  const auto& all_exec_ctxts = sim.GetAllExecCtxts();
  all_exec_ctxts[0]->SetupIterationAll(all_exec_ctxts);
  sim.GetEnvironment()->Update();

  auto agent0 = rm->GetAgent(AgentHandle(0, 0));

  auto for_each = L2F([&](Agent* agent, real_t squared_distance) {
    EXPECT_NE(0, squared_distance);
  });

  all_exec_ctxts[0]->ForEachNeighbor(for_each, *agent0, 400);

  // Once more to check the cached values
  all_exec_ctxts[0]->ForEachNeighbor(for_each, *agent0, 400);
}

}  // namespace in_place_exec_ctxt_detail
}  // namespace bdm
