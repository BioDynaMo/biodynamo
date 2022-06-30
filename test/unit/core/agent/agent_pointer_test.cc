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

#include "unit/core/agent/agent_pointer_test.h"
#include "core/randomized_rm.h"  // for bdm::Ubrng
#include "unit/test_util/io_test.h"

namespace bdm {
namespace agent_pointer_test_internal {

void RunBasicsTest(Simulation* simulation, AgentPointerMode mode) {
  auto prev_mode = gAgentPointerMode;
  gAgentPointerMode = mode;

  AgentPointer<TestAgent> null_agent_pointer;
  EXPECT_TRUE(null_agent_pointer == nullptr);

  TestAgent* agent = new TestAgent();
  agent->SetData(123);
  simulation->GetResourceManager()->AddAgent(agent);

  AgentPointer<TestAgent> agent_ptr(agent->GetUid());

  EXPECT_TRUE(agent_ptr != nullptr);
  EXPECT_TRUE(agent_ptr == agent);
  EXPECT_FALSE(agent_ptr != agent);

  EXPECT_EQ(123, agent_ptr->GetData());
  EXPECT_EQ(123, (*agent_ptr).GetData());
  EXPECT_EQ(123, agent_ptr.Get()->GetData());
  const AgentPointer<TestAgent>* const_agent_ptr = &agent_ptr;
  EXPECT_EQ(123, (*const_agent_ptr)->GetData());
  EXPECT_EQ(123, (*(*const_agent_ptr)).GetData());
  EXPECT_EQ(123, (*const_agent_ptr).Get()->GetData());

  TestAgent* so1 = new TestAgent();
  EXPECT_FALSE(agent_ptr == so1);
  EXPECT_TRUE(agent_ptr != so1);

  if (!agent_ptr) {
    FAIL();
  }

  agent_ptr = nullptr;
  EXPECT_TRUE(agent_ptr == nullptr);
  EXPECT_FALSE(agent_ptr == so1);

  if (agent_ptr) {
    FAIL();
  }

  delete so1;
  // restore gAgentPointerMode
  gAgentPointerMode = prev_mode;
}

TEST(AgentPointerTest, BasicsIndirect) {
  Simulation simulation(TEST_NAME);
  RunBasicsTest(&simulation, AgentPointerMode::kIndirect);
}

TEST(AgentPointerTest, BasicsDirect) {
  Simulation simulation(TEST_NAME);
  RunBasicsTest(&simulation, AgentPointerMode::kDirect);
}

void RunEqualsTest(AgentPointerMode mode) {
  auto prev_mode = gAgentPointerMode;
  gAgentPointerMode = mode;

  TestAgent ta1;
  TestAgent ta2;
  AgentPointer<Agent> null_ap = nullptr;
  auto ap1 = ta1.GetAgentPtr<>();
  auto ap2 = ta2.GetAgentPtr<>();

  EXPECT_FALSE(null_ap == ap1);
  EXPECT_FALSE(ap1 == null_ap);

  EXPECT_TRUE(ap1 == ap1);
  EXPECT_TRUE(ap2 == ta2.GetAgentPtr<>());

  TestAgent* null_ta = nullptr;

  EXPECT_TRUE(null_ap == null_ta);

  EXPECT_TRUE(ap2 == &ta2);

  // restore gAgentPointerMode
  gAgentPointerMode = prev_mode;
}

TEST(AgentPointerTest, EqualsIndirect) {
  Simulation simulation(TEST_NAME);
  RunEqualsTest(AgentPointerMode::kIndirect);
}

TEST(AgentPointerTest, EqualssDirect) {
  Simulation simulation(TEST_NAME);
  RunEqualsTest(AgentPointerMode::kDirect);
}

void RunSortTest(Simulation* sim, AgentPointerMode mode) {
  auto prev_mode = gAgentPointerMode;
  gAgentPointerMode = mode;

  auto* rm = sim->GetResourceManager();
  std::vector<AgentPointer<>> ap_vector(10);
  for (uint64_t i = 0u; i < ap_vector.size(); ++i) {
    rm->AddAgent(new TestAgent());
    ap_vector[i] = AgentPointer<>(AgentUid(i));
  }

  auto* random = Simulation::GetActive()->GetRandom();
  std::shuffle(ap_vector.begin(), ap_vector.end(), Ubrng(random));

  std::sort(ap_vector.begin(), ap_vector.end());
  for (uint64_t i = 0u; i < ap_vector.size(); ++i) {
    EXPECT_EQ(i, ap_vector[i].GetUid().GetIndex());
  }

  // restore gAgentPointerMode
  gAgentPointerMode = prev_mode;
}

TEST(AgentPointerTest, SortIndirect) {
  Simulation simulation(TEST_NAME);
  RunSortTest(&simulation, AgentPointerMode::kIndirect);
}

TEST(AgentPointerTest, SortDirect) {
  Simulation simulation(TEST_NAME);
  RunSortTest(&simulation, AgentPointerMode::kDirect);
}

void RunRemoveDuplicatesTest(Simulation* sim, AgentPointerMode mode) {
  auto prev_mode = gAgentPointerMode;
  gAgentPointerMode = mode;

  auto* rm = sim->GetResourceManager();
  std::vector<AgentPointer<>> ap_vector;
  ap_vector.reserve(1024);
  for (uint64_t i = 0u; i < 512; ++i) {
    rm->AddAgent(new TestAgent());
    ap_vector.push_back(AgentPointer<>(AgentUid(i)));
    ap_vector.push_back(AgentPointer<>(AgentUid(i)));
  }
  ap_vector.erase(std::unique(ap_vector.begin(), ap_vector.end()),
                  ap_vector.end());

  EXPECT_EQ(512u, ap_vector.size());
  for (uint64_t i = 0u; i < ap_vector.size(); ++i) {
    EXPECT_EQ(i, ap_vector[i].GetUid().GetIndex());
  }

  // restore gAgentPointerMode
  gAgentPointerMode = prev_mode;
}

TEST(AgentPointerTest, RemoveDuplicatesIndirect) {
  Simulation simulation(TEST_NAME);
  RunRemoveDuplicatesTest(&simulation, AgentPointerMode::kIndirect);
}

TEST(AgentPointerTest, RemoveDuplicatesDirect) {
  Simulation simulation(TEST_NAME);
  RunRemoveDuplicatesTest(&simulation, AgentPointerMode::kDirect);
}

void RunRemoveNullptrTest(Simulation* sim, AgentPointerMode mode) {
  auto prev_mode = gAgentPointerMode;
  gAgentPointerMode = mode;

  auto* rm = sim->GetResourceManager();
  rm->AddAgent(new TestAgent());
  rm->AddAgent(new TestAgent());
  rm->AddAgent(new TestAgent());

  std::vector<AgentPointer<>> ap_vector;
  ap_vector.push_back(AgentPointer<>(AgentUid(0)));
  ap_vector.push_back(AgentPointer<>(AgentUid(1)));
  ap_vector.push_back(AgentPointer<>(AgentUid(2)));
  ap_vector.push_back(AgentPointer<>(AgentUid()));
  ap_vector.push_back(AgentPointer<>(AgentUid()));

  while (ap_vector.size() && ap_vector.back() == nullptr) {
    ap_vector.pop_back();
  }
  EXPECT_EQ(3u, ap_vector.size());
  for (uint64_t i = 0u; i < ap_vector.size(); ++i) {
    EXPECT_EQ(i, ap_vector[i].GetUid().GetIndex());
  }

  // restore gAgentPointerMode
  gAgentPointerMode = prev_mode;
}

TEST(AgentPointerTest, RemoveNulltprIndirect) {
  Simulation simulation(TEST_NAME);
  RunRemoveNullptrTest(&simulation, AgentPointerMode::kIndirect);
}

TEST(AgentPointerTest, RemoveNulltprDirect) {
  Simulation simulation(TEST_NAME);
  RunRemoveNullptrTest(&simulation, AgentPointerMode::kDirect);
}

TEST(IsAgentPtrTest, All) {
  static_assert(!is_agent_ptr<TestAgent>::value,
                "TestAgent is not an AgentPointer");
  static_assert(is_agent_ptr<AgentPointer<TestAgent>>::value,
                "AgentPointer<TestAgent> is an AgentPointer");
}

#ifdef USE_DICT

TEST_F(IOTest, AgentPointerIndirect) {
  Simulation simulation(TEST_NAME);
  RunIOTest(&simulation, AgentPointerMode::kIndirect);
}

TEST_F(IOTest, AgentPointerDirect) {
  Simulation simulation(TEST_NAME);
  RunIOTest(&simulation, AgentPointerMode::kDirect);
}

TEST_F(IOTest, AgentPointerNullptrIndirect) {
  Simulation simulation(TEST_NAME);
  IOTestAgentPointerNullptr(AgentPointerMode::kIndirect);
}

TEST_F(IOTest, AgentPointerNullptrDirect) {
  Simulation simulation(TEST_NAME);
  IOTestAgentPointerNullptr(AgentPointerMode::kDirect);
}

#endif  // USE_DICT

}  // namespace agent_pointer_test_internal
}  // namespace bdm
