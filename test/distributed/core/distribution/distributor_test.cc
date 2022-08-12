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

#include "core/distribution/distributor.h"
#include <gtest/gtest.h>
#include <mpi.h>
#include "core/distribution/distribution_param.h"
#include "core/environment/environment.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/simulation_space.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace experimental {

TEST(SpatialSTKDistributor, Initialize) {
  auto set_param = [](Param* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -10;
    param->max_bound = 20;
    param->interaction_radius = 5;
    param->Get<DistributionParam>()->box_length_factor = 2;
  };

  Simulation simulation(TEST_NAME, set_param);
  auto* space = simulation.GetSimulationSpace();

  SimulationSpace::Space expected_ws = {-10, 20, -10, 20, -10, 20};
  EXPECT_EQ(expected_ws, space->GetWholeSpace());

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    SimulationSpace::Space expected_ls = {-10, 20, -10, 20, -10, 10};
    EXPECT_EQ(expected_ls, space->GetLocalSpace());
  } else {
    SimulationSpace::Space expected_ls = {-10, 20, -10, 20, 10, 20};
    EXPECT_EQ(expected_ls, space->GetLocalSpace());
  }
}

// -----------------------------------------------------------------------------
// Assumes that the space is partitioned like in
// SpatialSTKDistributor.Initialize
TEST(SpatialSTKDistributor, MigrateAgents) {
  auto set_param = [](Param* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -10;
    param->max_bound = 20;
    param->interaction_radius = 5;
    param->Get<DistributionParam>()->box_length_factor = 2;
  };

  Simulation simulation(TEST_NAME, set_param);
  auto* space = simulation.GetSimulationSpace();

  SimulationSpace::Space expected_ws = {-10, 20, -10, 20, -10, 20};
  EXPECT_EQ(expected_ws, space->GetWholeSpace());

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  // in each rank create an agent outside the local space
  if (rank == 0) {
    auto* agent = new TestAgent({0, 0, 15});
    agent->SetData(123);
    rm->AddAgent(agent);
  } else {
    auto* agent = new TestAgent({0, 0, 5});
    agent->SetData(321);
    rm->AddAgent(agent);
  }

  simulation.GetDistributor()->MigrateAgents();

  ASSERT_EQ(1, rm->GetNumAgents());

  auto* agent = bdm_static_cast<TestAgent*>(rm->GetAgent(AgentHandle(0, 0)));
  // verify that agents have been migrated to the right rank
  if (rank == 0) {
    EXPECT_EQ(321, agent->GetData());
    Real3 expected_position = {0, 0, 5};
    EXPECT_EQ(expected_position, agent->GetPosition());
  } else {
    EXPECT_EQ(123, agent->GetData());
    Real3 expected_position = {0, 0, 15};
    EXPECT_EQ(expected_position, agent->GetPosition());
  }
}

// -----------------------------------------------------------------------------
// Assumes that the space is partitioned like in
// SpatialSTKDistributor.Initialize
TEST(SpatialSTKDistributor, SendAuraFirstTime) {
  auto set_param = [](Param* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -10;
    param->max_bound = 20;
    param->interaction_radius = 5;
    param->Get<DistributionParam>()->box_length_factor = 2;
  };

  Simulation simulation(TEST_NAME, set_param);
  auto* space = simulation.GetSimulationSpace();
  auto* env = simulation.GetEnvironment();

  SimulationSpace::Space expected_ws = {-10, 20, -10, 20, -10, 20};
  EXPECT_EQ(expected_ws, space->GetWholeSpace());

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  // in each rank create an agent outside the local space
  if (rank == 0) {
    auto* agent = new TestAgent({0, 0, 5});
    agent->SetData(123);
    rm->AddAgent(agent);
  } else {
    auto* agent = new TestAgent({0, 0, 15});
    agent->SetData(321);
    rm->AddAgent(agent);
  }

  simulation.GetDistributor()->UpdateAura();
  env->Update();

  ASSERT_EQ(1, rm->GetNumAgents());

  std::vector<Agent*> neighbors;
  auto fen = L2F([&](Agent* agent, real_t) { neighbors.push_back(agent); });

  auto* agent = bdm_static_cast<TestAgent*>(rm->GetAgent(AgentHandle(0, 0)));
  env->ForEachNeighbor(fen, *agent, 10);

  ASSERT_EQ(1, neighbors.size());
  auto* neighbor = static_cast<TestAgent*>(neighbors[0]);

  // verify that agents find the neighbor in the aura region
  if (rank == 0) {
    EXPECT_EQ(321, neighbor->GetData());
    Real3 expected_position = {0, 0, 5};
    EXPECT_EQ(expected_position, neighbor->GetPosition());
  } else {
    EXPECT_EQ(123, neighbor->GetData());
    Real3 expected_position = {0, 0, 15};
    EXPECT_EQ(expected_position, neighbor->GetPosition());
  }
}

}  // namespace experimental
}  // namespace bdm
