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

#include "unit/core/agent/agent_pointer_test.h"
#include "unit/test_util/io_test.h"

namespace bdm {
namespace agent_pointer_test_internal {

TEST(SoPointerTest, Basics) {
  Simulation simulation(TEST_NAME);

  AgentPointer<TestAgent> null_agent_pointer;
  EXPECT_TRUE(null_agent_pointer == nullptr);

  TestAgent* agent = new TestAgent();
  agent->SetData(123);
  simulation.GetResourceManager()->push_back(agent);

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
}

TEST(IsAgentPtrTest, All) {
  static_assert(!is_agent_ptr<TestAgent>::value,
                "TestAgent is not an AgentPointer");
  static_assert(is_agent_ptr<AgentPointer<TestAgent>>::value,
                "AgentPointer<TestAgent> is an AgentPointer");
}

#ifdef USE_DICT

TEST_F(IOTest, AgentPointer) {
  Simulation simulation(TEST_NAME);
  RunIOTest(&simulation);
}

TEST_F(IOTest, SoPointerNullptr) {
  Simulation simulation(TEST_NAME);
  IOTestSoPointerNullptr();
}

#endif  // USE_DICT

}  // namespace agent_pointer_test_internal
}  // namespace bdm
