// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "core/randomized_rm.h"
#include <gtest/gtest.h>
#include "core/functor.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(RandomizedRm, Basics) {
  Simulation simulation(TEST_NAME);
  int n = 10;
  auto* rm = new RandomizedRm<ResourceManager>();
  simulation.SetResourceManager(rm);

  for (int i = 0; i < n; i++) {
    rm->AddAgent(new TestAgent(i));
  }

  std::vector<int> called;
  auto functor = L2F([&](Agent* a, AgentHandle) {
    called.push_back(bdm_static_cast<TestAgent*>(a)->GetData());
  });
  rm->ForEachAgent(functor);
  EXPECT_EQ(static_cast<uint64_t>(n), called.size());
  for (uint64_t i = 0; i < called.size(); i++) {
    EXPECT_EQ(static_cast<int>(i), called[i]);
  }

  // trigger randomization
  rm->EndOfIteration();

  auto called_copy = called;
  called.clear();
  rm->ForEachAgent(functor);
  EXPECT_TRUE(called_copy != called);
  EXPECT_EQ(static_cast<uint64_t>(n), called.size());
  auto called_copy1 = called;

  // check that every entry is unique
  std::sort(called.begin(), called.end());
  for (uint64_t i = 1; i < called.size(); ++i) {
    EXPECT_TRUE(called[i - 1] != called[i]);
  }

  // check if agent uid map points to correct agent
  for (int i = 0; i < n; ++i) {
    AgentUid uid(i);
    auto* a = bdm_static_cast<TestAgent*>(rm->GetAgent(uid));
    EXPECT_EQ(i, a->GetData());
  }

  // check if results are stable within the same iteration
  called.clear();
  rm->ForEachAgent(functor);
  EXPECT_EQ(called_copy1, called);
}

}  // namespace bdm
