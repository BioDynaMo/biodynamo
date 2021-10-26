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

#ifndef UNIT_CORE_AGENT_AGENT_POINTER_TEST_H_
#define UNIT_CORE_AGENT_AGENT_POINTER_TEST_H_

#include <gtest/gtest.h>

#include "core/agent/agent_pointer.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_agent.h"

namespace bdm {
namespace agent_pointer_test_internal {

inline void RunIOTest(Simulation* sim) {
  auto* rm = sim->GetResourceManager();
  rm->AddAgent(new TestAgent(123));
  TestAgent* so2 = new TestAgent(456);
  rm->AddAgent(so2);

  AgentPointer<TestAgent> agent_ptr(so2->GetUid());
  AgentPointer<TestAgent>* restored;

  BackupAndRestore(agent_ptr, &restored);

  EXPECT_TRUE(*restored != nullptr);
  EXPECT_EQ(456, (*restored)->GetData());
}

inline void IOTestSoPointerNullptr() {
  AgentPointer<TestAgent> null_agent_pointer;
  AgentPointer<TestAgent>* restored = nullptr;

  BackupAndRestore(null_agent_pointer, &restored);

  EXPECT_TRUE(*restored == nullptr);
}

}  // namespace agent_pointer_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_AGENT_AGENT_POINTER_TEST_H_
