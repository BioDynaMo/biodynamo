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

#ifndef UNIT_CORE_RESOURCE_MANAGER_TEST_H_
#define UNIT_CORE_RESOURCE_MANAGER_TEST_H_

#include <algorithm>
#include <vector>
#include "core/environment/environment.h"
#include "core/resource_manager.h"
#include "core/agent/agent.h"
#include "core/util/io.h"
#include "core/util/type.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

class A : public TestAgent {
  BDM_AGENT_HEADER(A, TestAgent, 1);

 public:
  A() {}  // for ROOT I/O
  A(const Event& event, Agent* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}
  explicit A(int data) { data_ = data; }

  int GetData() const { return data_; }
  void SetData(int data) { data_ = data; }

  int data_;
};

class B : public TestAgent {
  BDM_AGENT_HEADER(B, TestAgent, 1);

 public:
  B() {}  // for ROOT I/O
  B(const Event& event, Agent* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}
  explicit B(double data) { data_ = data; }

  double GetData() const { return data_; }
  void SetData(double data) { data_ = data; }

  double data_;
};

inline void RunForEachAgentTest() {
  const double kEpsilon = abs_error<double>::value;
  Simulation simulation("RunForEachAgentTest");
  auto* rm = simulation.GetResourceManager();

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());

  rm->AddAgent(new A(12));
  rm->AddAgent(new A(34));

  rm->AddAgent(new B(3.14));
  rm->AddAgent(new B(6.28));
  uint64_t counter = 0;
  rm->ForEachAgent([&](Agent* element) {  // NOLINT
    counter++;
    switch (element->GetUid() - ref_uid) {
      case 0:
        EXPECT_EQ(12, dynamic_cast<A*>(element)->GetData());
        break;
      case 1:
        EXPECT_EQ(34, dynamic_cast<A*>(element)->GetData());
        break;
      case 2:
        EXPECT_NEAR(3.14, dynamic_cast<B*>(element)->GetData(), kEpsilon);
        break;
      case 3:
        EXPECT_NEAR(6.28, dynamic_cast<B*>(element)->GetData(), kEpsilon);
        break;
    }
  });

  EXPECT_EQ(4u, counter);
}

inline void RunGetNumAgents() {
  Simulation simulation("ResourceManagerTest-RunGetNumAgents");
  auto* rm = simulation.GetResourceManager();

  rm->AddAgent(new A(12));
  rm->AddAgent(new A(34));
  rm->AddAgent(new A(59));

  rm->AddAgent(new B(3.14));
  rm->AddAgent(new B(6.28));

  EXPECT_EQ(5u, rm->GetNumAgents());
}

struct ForEachAgentParallelTestFunctor : Functor<void, Agent*> {
  void operator()(Agent* agent) override {
    const double kEpsilon = abs_error<double>::value;
    B* b = dynamic_cast<B*>(agent);
    AgentUid uid = agent->GetUid();
    if (uid == AgentUid(0)) {
      EXPECT_EQ(3.14, b->GetData());
    } else if (uid == AgentUid(1)) {
      EXPECT_EQ(6.28, b->GetData());
    } else if (uid == AgentUid(2)) {
      EXPECT_NEAR(9.42, b->GetData(), kEpsilon);
    } else {
      FAIL();
    }
  }
};

// This test uses Cells since A, and B are strippted down agents
// and are themselves not thread safe.
inline void RunForEachAgentParallelTest() {
  Simulation simulation("RunForEachAgentParallelTest");
  auto* rm = simulation.GetResourceManager();

  rm->AddAgent(new B(3.14));
  rm->AddAgent(new B(6.28));
  rm->AddAgent(new B(9.42));

  ForEachAgentParallelTestFunctor functor;
  rm->ForEachAgentParallel(functor);
}

inline void RunRemoveAndContainsTest() {
  Simulation simulation("ResourceManagerTest-RunRemoveAndContainsTest");
  auto* rm = simulation.GetResourceManager();

  A* a0 = new A(12);
  auto a0_uid = a0->GetUid();
  rm->AddAgent(a0);

  A* a1 = new A(34);
  auto a1_uid = a1->GetUid();
  rm->AddAgent(a1);

  A* a2 = new A(59);
  auto a2_uid = a2->GetUid();
  rm->AddAgent(a2);

  B* b0 = new B(3.14);
  auto b0_uid = b0->GetUid();
  rm->AddAgent(b0);

  B* b1 = new B(6.28);
  auto b1_uid = b1->GetUid();
  rm->AddAgent(b1);

  EXPECT_TRUE(rm->ContainsAgent(a0_uid));
  EXPECT_TRUE(rm->ContainsAgent(a1_uid));
  EXPECT_TRUE(rm->ContainsAgent(a2_uid));
  EXPECT_TRUE(rm->ContainsAgent(b0_uid));
  EXPECT_TRUE(rm->ContainsAgent(b1_uid));

  rm->RemoveAgent(a0_uid);
  rm->RemoveAgent(a1_uid);
  rm->RemoveAgent(a2_uid);
  rm->RemoveAgent(b0_uid);
  rm->RemoveAgent(b1_uid);

  EXPECT_FALSE(rm->ContainsAgent(a0_uid));
  EXPECT_FALSE(rm->ContainsAgent(a1_uid));
  EXPECT_FALSE(rm->ContainsAgent(a2_uid));
  EXPECT_FALSE(rm->ContainsAgent(b0_uid));
  EXPECT_FALSE(rm->ContainsAgent(b1_uid));

  EXPECT_EQ(0u, rm->GetNumAgents());
}

inline void RunClearTest() {
  Simulation simulation("ResourceManagerTest-RunClearTest");
  auto* rm = simulation.GetResourceManager();

  A* a0 = new A(12);
  auto a0_uid = a0->GetUid();
  rm->AddAgent(a0);

  A* a1 = new A(34);
  auto a1_uid = a1->GetUid();
  rm->AddAgent(a1);

  A* a2 = new A(59);
  auto a2_uid = a2->GetUid();
  rm->AddAgent(a2);

  B* b0 = new B(3.14);
  auto b0_uid = b0->GetUid();
  rm->AddAgent(b0);

  B* b1 = new B(6.28);
  auto b1_uid = b1->GetUid();
  rm->AddAgent(b1);

  EXPECT_TRUE(rm->ContainsAgent(a0_uid));
  EXPECT_TRUE(rm->ContainsAgent(a1_uid));
  EXPECT_TRUE(rm->ContainsAgent(a2_uid));
  EXPECT_TRUE(rm->ContainsAgent(b0_uid));
  EXPECT_TRUE(rm->ContainsAgent(b1_uid));

  rm->ClearAgents();

  EXPECT_FALSE(rm->ContainsAgent(a0_uid));
  EXPECT_FALSE(rm->ContainsAgent(a1_uid));
  EXPECT_FALSE(rm->ContainsAgent(a2_uid));
  EXPECT_FALSE(rm->ContainsAgent(b0_uid));
  EXPECT_FALSE(rm->ContainsAgent(b1_uid));

  EXPECT_EQ(0u, rm->GetNumAgents());
}

inline void RunPushBackAndGetAgentTest() {
  const double kEpsilon = abs_error<double>::value;
  Simulation simulation("RunPushBackAndGetAgentTest");
  auto* rm = simulation.GetResourceManager();

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());

  rm->AddAgent(new A(12));
  rm->AddAgent(new A(34));

  rm->AddAgent(new B(3.14));
  rm->AddAgent(new B(6.28));

  rm->AddAgent(new A(87));

  EXPECT_EQ(dynamic_cast<A*>(rm->GetAgent(ref_uid))->GetData(), 12);
  EXPECT_EQ(dynamic_cast<A*>(rm->GetAgent(ref_uid + 1))->GetData(), 34);
  EXPECT_EQ(dynamic_cast<A*>(rm->GetAgent(ref_uid + 4))->GetData(), 87);

  EXPECT_NEAR(dynamic_cast<B*>(rm->GetAgent(ref_uid + 2))->GetData(), 3.14,
              kEpsilon);
  EXPECT_NEAR(dynamic_cast<B*>(rm->GetAgent(ref_uid + 3))->GetData(), 6.28,
              kEpsilon);
}

// -----------------------------------------------------------------------------
// https://github.com/osmhpi/pgasus/blob/775a5f90d8f6fa89cfb93eac6de16dcfe27167ce/src/util/mmaphelper.cpp
inline static void* AlignPage(const void* ptr) {
  static constexpr uintptr_t kPageMask = ~(uintptr_t(0xFFF));
  return (void*)(((uintptr_t)ptr) & kPageMask);  // NOLINT
}

inline int GetNumaNodeForMemory(const void* ptr) {
  int result, loc;
  void* pptr = AlignPage(ptr);
  result = numa_move_pages(0, 1, &pptr, nullptr, &loc, 0);
  return (result != 0) ? -1 : loc;
}

inline std::vector<uint64_t> GetAgentsPerNuma(uint64_t num_agents) {
  // balance agents per numa node according to the number of
  // threads associated with each numa domain
  auto* ti = ThreadInfo::GetInstance();
  int numa_nodes = ti->GetNumaNodes();

  std::vector<uint64_t> agent_per_numa(numa_nodes);
  uint64_t cummulative = 0;
  auto max_threads = ti->GetMaxThreads();
  for (int n = 1; n < numa_nodes; ++n) {
    auto threads_in_numa = ti->GetThreadsInNumaNode(n);
    uint64_t num_agents_loc = num_agents * threads_in_numa / max_threads;
    agent_per_numa[n] = num_agents_loc;
    cummulative += num_agents_loc;
  }
  agent_per_numa[0] = num_agents - cummulative;
  return agent_per_numa;
}

// -----------------------------------------------------------------------------
struct CheckForEachAgentFunctor : Functor<void, Agent*> {
  bool numa_checks;
  std::vector<bool> found;
  std::atomic<uint64_t> cnt;
  // counts the number of agents in each numa domain
  std::vector<uint64_t> numa_agent_cnts;
  std::atomic<uint64_t> numa_memory_errors;
  std::atomic<uint64_t> numa_thread_errors;

  CheckForEachAgentFunctor(uint64_t num_agent_per_type, bool numa_checks)
      : numa_checks(numa_checks),
        cnt(0),
        numa_memory_errors(0),
        numa_thread_errors(0) {
    found.resize(2 * num_agent_per_type);
    for (uint64_t i = 0; i < found.size(); ++i) {
      found[i] = false;
    }

    auto* ti = ThreadInfo::GetInstance();
    numa_agent_cnts.resize(ti->GetNumaNodes());
  }

  void operator()(Agent* agent) override {
    size_t index = 0;
    if (A* a = dynamic_cast<A*>(agent)) {
      index = a->GetData();
    } else if (B* b = dynamic_cast<B*>(agent)) {
      index = std::round(b->GetData());
    }
    auto* rm = Simulation::GetActive()->GetResourceManager();
    auto handle = rm->GetAgentHandle(agent->GetUid());

#pragma omp critical
    {
      found[index] = true;

      // verify that a thread processes agents on the same NUMA node.
      if (numa_checks && handle.GetNumaNode() != GetNumaNodeForMemory(agent)) {
        numa_memory_errors++;
      }
      if (numa_checks &&
          handle.GetNumaNode() != numa_node_of_cpu(sched_getcpu())) {
        numa_thread_errors++;
      }

      numa_agent_cnts[handle.GetNumaNode()]++;
    }
    cnt++;
  }
};

inline void CheckForEachAgent(ResourceManager* rm,
                                    uint64_t num_agent_per_type,
                                    bool numa_checks = false) {
  CheckForEachAgentFunctor functor(num_agent_per_type, numa_checks);
  rm->ForEachAgentParallel(functor);

  EXPECT_EQ(2 * num_agent_per_type, functor.cnt.load());
  ASSERT_EQ(2 * num_agent_per_type, functor.found.size());
  for (uint64_t i = 0; i < functor.found.size(); ++i) {
    if (!functor.found[i]) {
      FAIL()
          << "ForEachAgentParallel was not called for element with data_="
          << i;
    }
  }

  if (numa_checks) {
    EXPECT_EQ(0u, functor.numa_memory_errors.load());
    EXPECT_EQ(0u, functor.numa_thread_errors.load());
    auto agent_per_numa = GetAgentsPerNuma(2 * num_agent_per_type);
    auto* ti = ThreadInfo::GetInstance();
    for (int n = 0; n < ti->GetNumaNodes(); ++n) {
      EXPECT_EQ(agent_per_numa[n], functor.numa_agent_cnts[n]);
    }
  }
}

inline void RunSortAndForEachAgentParallel(uint64_t num_agent_per_type) {
  Simulation simulation("RunSortAndForEachAgentParallel");
  auto* rm = simulation.GetResourceManager();

  std::unordered_map<AgentUid, double> a_x_values;
  std::unordered_map<AgentUid, double> b_x_values;
  for (uint64_t i = 0; i < num_agent_per_type; ++i) {
    double x_pos = i * 30.0;

    A* a = new A(i);
    a->SetDiameter(10);
    a->SetPosition({x_pos, 0, 0});
    rm->AddAgent(a);
    a_x_values[a->GetUid()] = x_pos;

    B* b = new B(i + num_agent_per_type);
    b->SetDiameter(10);
    b->SetPosition({x_pos, 0, 0});
    rm->AddAgent(b);
    b_x_values[b->GetUid()] = x_pos;
  }

  CheckForEachAgent(rm, num_agent_per_type);

  simulation.GetEnvironment()->Update();
  rm->LoadBalance();

  CheckForEachAgent(rm, num_agent_per_type, true);

  // check if agent uids still point to the correct object
  for (auto& entry : a_x_values) {
    auto x_actual = rm->GetAgent(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
  for (auto& entry : b_x_values) {
    auto x_actual = rm->GetAgent(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
}

inline void RunSortAndForEachAgentParallel() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_agent_per_type = {std::max(1, num_threads - 1), num_threads,
                                      3 * num_threads, 3 * num_threads + 1};

  for (auto n : num_agent_per_type) {
    RunSortAndForEachAgentParallel(n);
  }

  RunSortAndForEachAgentParallel(1000);
}

// -----------------------------------------------------------------------------
struct CheckForEachAgentDynamicFunctor
    : Functor<void, Agent*, AgentHandle> {
  CheckForEachAgentDynamicFunctor(bool numa_checks,
                                        std::vector<bool>& found)
      : numa_checks_(numa_checks),
        found_(found),
        cnt(0),
        numa_memory_errors(0) {
    auto* ti = ThreadInfo::GetInstance();
    numa_agent_cnts.resize(ti->GetNumaNodes());
  }
  void operator()(Agent* agent, AgentHandle handle) override {
#pragma omp critical
    {
      size_t index = 0;
      if (A* a = dynamic_cast<A*>(agent)) {
        index = a->GetData();
      } else if (B* b = dynamic_cast<B*>(agent)) {
        index = std::round(b->GetData());
      }
      found_[index] = true;

      // verify that a thread processes agents on the same NUMA node.
      if (numa_checks_ && handle.GetNumaNode() != GetNumaNodeForMemory(agent)) {
        numa_memory_errors++;
      }

      numa_agent_cnts[handle.GetNumaNode()]++;
    }
    cnt++;
  }

  bool numa_checks_;
  std::vector<bool>& found_;

  std::atomic<uint64_t> cnt;
  // counts the number of agents in each numa domain
  std::vector<uint64_t> numa_agent_cnts;
  // If an agent is not stored on the NUMA indicated, it is a memory
  // error.
  std::atomic<uint64_t> numa_memory_errors;
};

struct CheckNumaThreadErrors : Functor<void, Agent*, AgentHandle> {
  CheckNumaThreadErrors() : numa_thread_errors(0) {
    ti_ = ThreadInfo::GetInstance();
  }

  void operator()(Agent* agent, AgentHandle handle) override {
    volatile double d = 0;
    for (int i = 0; i < 10000; i++) {
      d += std::sin(i);
    }
    if (handle.GetNumaNode() != ti_->GetNumaNode(omp_get_thread_num())) {
      numa_thread_errors++;
    }
  }

  // If an agent is processed by a thread that doesn't belong to the NUMA
  // domain the agent is stored on, it is a thread error.
  std::atomic<uint64_t> numa_thread_errors;
  ThreadInfo* ti_;
};

inline void CheckForEachAgentDynamic(ResourceManager* rm,
                                           uint64_t num_agent_per_type,
                                           uint64_t batch_size,
                                           bool numa_checks = false) {
  std::vector<bool> found(2 * num_agent_per_type);
  ASSERT_EQ(2 * num_agent_per_type, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    found[i] = false;
  }

  auto* ti = ThreadInfo::GetInstance();

  CheckForEachAgentDynamicFunctor functor(numa_checks, found);
  rm->ForEachAgentParallel(batch_size, functor);

  // critical sections increase the variance of numa_thread_errors.
  // Therefore, there are checked separately.
  CheckNumaThreadErrors check_numa_thread_functor;
  rm->ForEachAgentParallel(batch_size, check_numa_thread_functor);

  // verify that the function has been called once for each agent
  EXPECT_EQ(2 * num_agent_per_type, functor.cnt.load());
  ASSERT_EQ(2 * num_agent_per_type, found.size());
  for (uint64_t i = 0; i < found.size(); ++i) {
    if (!found[i]) {
      FAIL()
          << "ForEachAgentParallel was not called for element with data_="
          << i;
    }
  }

  if (numa_checks) {
    // If there are memory errors, check of
    // `cat /proc/sys/kernel/numa_balancing` is zero.
    // Automatic rebalancing can lead to numa memory errors.
    // only 0.1% of all agents may be on a wrong numa node
    EXPECT_GT(0.001, (functor.numa_memory_errors.load() + 0.0) /
                         (2 * num_agent_per_type));
    // work stealing can cause thread errors. This check ensures that at least
    // 75% of the work is done by the correct CPU-Memory mapping.
    if (num_agent_per_type > 20 * static_cast<uint64_t>(omp_get_max_threads())) {
      EXPECT_GT(num_agent_per_type / 4,
                check_numa_thread_functor.numa_thread_errors.load());
    }
    auto agent_per_numa = GetAgentsPerNuma(2 * num_agent_per_type);
    for (int n = 0; n < ti->GetNumaNodes(); ++n) {
      EXPECT_EQ(agent_per_numa[n], functor.numa_agent_cnts[n]);
    }
  }
}

inline void RunSortAndForEachAgentParallelDynamic(
    uint64_t num_agent_per_type, uint64_t batch_size) {
  Simulation simulation("RunSortAndForEachAgentParallelDynamic");
  auto* rm = simulation.GetResourceManager();

  std::unordered_map<AgentUid, double> a_x_values;
  std::unordered_map<AgentUid, double> b_x_values;
  for (uint64_t i = 0; i < num_agent_per_type; ++i) {
    double x_pos = i * 30.0;

    A* a = new A(i);
    a->SetDiameter(10);
    a->SetPosition({x_pos, 0, 0});
    rm->AddAgent(a);
    a_x_values[a->GetUid()] = x_pos;

    B* b = new B(i + num_agent_per_type);
    b->SetDiameter(10);
    b->SetPosition({x_pos, 0, 0});
    rm->AddAgent(b);
    b_x_values[b->GetUid()] = x_pos;
  }

  CheckForEachAgentDynamic(rm, num_agent_per_type, batch_size);

  simulation.GetEnvironment()->Update();
  rm->LoadBalance();

  CheckForEachAgentDynamic(rm, num_agent_per_type, batch_size, true);

  // check if agent uids still point to the correct object
  for (auto& entry : a_x_values) {
    auto x_actual = rm->GetAgent(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
  for (auto& entry : b_x_values) {
    auto x_actual = rm->GetAgent(entry.first)->GetPosition()[0];
    EXPECT_EQ(x_actual, entry.second);
  }
}

inline void RunSortAndForEachAgentParallelDynamic() {
  int num_threads = omp_get_max_threads();
  std::vector<int> num_agent_per_type = {std::max(1, num_threads - 1), num_threads,
                                      3 * num_threads, 3 * num_threads + 1};
  std::vector<int> batch_sizes = {std::max(1, num_threads - 1), num_threads,
                                  3 * num_threads, 3 * num_threads + 1};

  for (auto n : num_agent_per_type) {
    for (auto b : batch_sizes) {
      RunSortAndForEachAgentParallelDynamic(n, b);
    }
  }

  for (auto b : batch_sizes) {
    RunSortAndForEachAgentParallelDynamic(num_threads * 1000, b);
  }
}

inline void RunIOTest() {
  const double kEpsilon = abs_error<double>::value;
  Simulation simulation("ResourceManagerTest-RunIOTest");
  auto* rm = simulation.GetResourceManager();

  auto ref_uid = AgentUid(simulation.GetAgentUidGenerator()->GetHighestIndex());
  remove(ROOTFILE);

  // setup
  rm->AddAgent(new A(12));
  rm->AddAgent(new A(34));
  rm->AddAgent(new A(42));

  rm->AddAgent(new B(3.14));
  rm->AddAgent(new B(6.28));

  DiffusionGrid* dgrid_1 = new DiffusionGrid(0, "Kalium", 0.4, 0, 2);
  DiffusionGrid* dgrid_2 = new DiffusionGrid(1, "Natrium", 0.2, 0.1, 1);
  rm->AddDiffusionGrid(dgrid_1);
  rm->AddDiffusionGrid(dgrid_2);

  // backup
  WritePersistentObject(ROOTFILE, "rm", *rm, "new");

  rm->ClearAgents();

  // restore
  ResourceManager* restored_rm = nullptr;
  GetPersistentObject(ROOTFILE, "rm", restored_rm);
  restored_rm->RebuildAgentUidMap();

  // validate
  EXPECT_EQ(5u, restored_rm->GetNumAgents());

  EXPECT_EQ(12,
            dynamic_cast<A*>(restored_rm->GetAgent(ref_uid))->GetData());
  EXPECT_EQ(
      34, dynamic_cast<A*>(restored_rm->GetAgent(ref_uid + 1))->GetData());
  EXPECT_EQ(
      42, dynamic_cast<A*>(restored_rm->GetAgent(ref_uid + 2))->GetData());

  EXPECT_NEAR(
      3.14, dynamic_cast<B*>(restored_rm->GetAgent(ref_uid + 3))->GetData(),
      kEpsilon);
  EXPECT_NEAR(
      6.28, dynamic_cast<B*>(restored_rm->GetAgent(ref_uid + 4))->GetData(),
      kEpsilon);

  EXPECT_EQ(0, restored_rm->GetDiffusionGrid(0)->GetSubstanceId());
  EXPECT_EQ(1, restored_rm->GetDiffusionGrid(1)->GetSubstanceId());
  EXPECT_EQ("Kalium", restored_rm->GetDiffusionGrid(0)->GetSubstanceName());
  EXPECT_EQ("Natrium", restored_rm->GetDiffusionGrid(1)->GetSubstanceName());
  EXPECT_EQ(0.6,
            restored_rm->GetDiffusionGrid(0)->GetDiffusionCoefficients()[0]);
  EXPECT_EQ(0.8,
            restored_rm->GetDiffusionGrid(1)->GetDiffusionCoefficients()[0]);

  delete restored_rm;

  remove(ROOTFILE);
}

}  // namespace bdm

#endif  // UNIT_CORE_RESOURCE_MANAGER_TEST_H_
