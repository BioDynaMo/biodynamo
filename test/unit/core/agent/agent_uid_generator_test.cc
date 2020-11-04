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

#include "core/agent/agent_uid_generator.h"
#include <gtest/gtest.h>
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_agent.h"

namespace bdm {

TEST(AgentUidGeneratorTest, NormalAndDefragmentationMode) {
  Simulation simulation(TEST_NAME);
  simulation.GetResourceManager()->push_back(new TestAgent(0));

  AgentUidGenerator generator;

  EXPECT_EQ(AgentUid(0), generator.NewAgentUid());
  EXPECT_EQ(AgentUid(1), generator.NewAgentUid());
  EXPECT_EQ(AgentUid(2), generator.NewAgentUid());

  // increment simulated time steps
  simulation.GetScheduler()->Simulate(1);

  // defragmentation mode
  AgentUidMap<AgentHandle> map(3);
  map.Insert(AgentUid(1), AgentHandle(123));
  // slots 0, and 2 are empty

  generator.EnableDefragmentation(&map);
  EXPECT_EQ(AgentUid(0, 1), generator.NewAgentUid());
  EXPECT_EQ(AgentUid(2, 1), generator.NewAgentUid());
  // no more empty slots -> generator should have switched back to normal mode
  EXPECT_EQ(AgentUid(3, 0), generator.NewAgentUid());
}

#ifdef USE_DICT
TEST_F(IOTest, AgentUidGenerator) {
  AgentUidGenerator test;
  test.NewAgentUid();
  test.NewAgentUid();
  test.NewAgentUid();

  AgentUidGenerator* restored = nullptr;

  BackupAndRestore(test, &restored);

  EXPECT_EQ(restored->GetHighestIndex(), 3u);
  EXPECT_EQ(restored->NewAgentUid(), AgentUid(3u));

  delete restored;
}
#endif  // USE_DICT

}  // namespace bdm
