// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include "core/container/agent_vector.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_agent.h"

namespace bdm {

TEST(AgentVectorTest, All) {
  std::string sim_name("simulation_object_vector_test_RunInitializerTest");
  Simulation simulation(sim_name);
  auto* rm = simulation.GetResourceManager();

  rm->AddAgent(new TestAgent());
  rm->AddAgent(new TestAgent());
  rm->AddAgent(new TestAgent());

  AgentVector<int> vector;
  EXPECT_EQ(3u, vector.size(0));

  // values are not initialized
  vector[AgentHandle(0, 0)] = 1;
  vector[AgentHandle(0, 1)] = 2;
  vector[AgentHandle(0, 2)] = 3;

  EXPECT_EQ(1, vector[AgentHandle(0, 0)]);
  EXPECT_EQ(2, vector[AgentHandle(0, 1)]);
  EXPECT_EQ(3, vector[AgentHandle(0, 2)]);

  vector.clear();
  EXPECT_EQ(0u, vector.size(0));
}

TEST(AgentVectorTest, Equality) {
  std::string sim_name("simulation_object_vector_test_RunInitializerTest2");
  Simulation simulation(sim_name);
  auto* rm = simulation.GetResourceManager();

  rm->AddAgent(new TestAgent());
  rm->AddAgent(new TestAgent());
  rm->AddAgent(new TestAgent());

  AgentVector<int> vec_a;
  AgentVector<int> vec_b;
  EXPECT_EQ(3u, vec_a.size(0));
  EXPECT_EQ(3u, vec_b.size(0));

  // values are not initialized
  vec_a[AgentHandle(0, 0)] = 1;
  vec_a[AgentHandle(0, 1)] = 2;
  vec_a[AgentHandle(0, 2)] = 3;

  EXPECT_NE(vec_a, vec_b);

  vec_b[AgentHandle(0, 0)] = 1;
  vec_b[AgentHandle(0, 1)] = 2;
  vec_b[AgentHandle(0, 2)] = 3;

  EXPECT_EQ(vec_a, vec_b);

  vec_a.clear();
  vec_b.clear();
  EXPECT_EQ(vec_a, vec_b);
}

}  // namespace bdm
