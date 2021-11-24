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

#include "core/environment/kd_tree_environment.h"
#include "core/agent/cell.h"
#include "gtest/gtest.h"
#include "unit/core/count_neighbor_functor.h"
#include "unit/test_util/test_util.h"

namespace bdm {

inline void CellFactory(ResourceManager* rm, size_t cells_per_dim) {
  const double space = 20;
  rm->Reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell* cell = new Cell({k * space, j * space, i * space});
        cell->SetDiameter(30);
        rm->AddAgent(cell);
      }
    }
  }
}

struct FillNeighborList : public Functor<void, Agent*, double> {
  std::unordered_map<AgentUid, std::vector<AgentUid>>* neighbors_;
  AgentUid uid_;
  FillNeighborList(
      std::unordered_map<AgentUid, std::vector<AgentUid>>* neighbors,
      AgentUid uid)
      : neighbors_(neighbors), uid_(uid) {}

  void operator()(Agent* neighbor, double squared_distance) override {
    auto nuid = neighbor->GetUid();
    if (uid_ != nuid) {
      (*neighbors_)[uid_].push_back(nuid);
    }
  }
};

TEST(KDTreeTest, Setup) {
  auto set_param = [](auto* param) { param->environment = "kd_tree"; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* kdtree = dynamic_cast<KDTreeEnvironment*>(simulation.GetEnvironment());

  // Check if our environment is in fact a KD tree
  EXPECT_NE(kdtree, nullptr);

  CellFactory(rm, 4);

  kdtree->Update();

  std::unordered_map<AgentUid, std::vector<AgentUid>> neighbors;
  neighbors.reserve(rm->GetNumAgents());

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  double search_radius_squared = 1201;
  rm->ForEachAgent([&](Agent* so) {
    auto uid = so->GetUid();
    FillNeighborList fill_neighbor_list(&neighbors, uid);
    kdtree->ForEachNeighbor(fill_neighbor_list, *so, search_radius_squared);
  });

  std::vector<AgentUid> expected_0 = {AgentUid(1),  AgentUid(4),  AgentUid(5),
                                      AgentUid(16), AgentUid(17), AgentUid(20),
                                      AgentUid(21)};
  std::vector<AgentUid> expected_4 = {AgentUid(0),  AgentUid(1),  AgentUid(5),
                                      AgentUid(8),  AgentUid(9),  AgentUid(16),
                                      AgentUid(17), AgentUid(20), AgentUid(21),
                                      AgentUid(24), AgentUid(25)};
  std::vector<AgentUid> expected_42 = {
      AgentUid(21), AgentUid(22), AgentUid(23), AgentUid(25), AgentUid(26),
      AgentUid(27), AgentUid(29), AgentUid(30), AgentUid(31), AgentUid(37),
      AgentUid(38), AgentUid(39), AgentUid(41), AgentUid(43), AgentUid(45),
      AgentUid(46), AgentUid(47), AgentUid(53), AgentUid(54), AgentUid(55),
      AgentUid(57), AgentUid(58), AgentUid(59), AgentUid(61), AgentUid(62),
      AgentUid(63)};
  std::vector<AgentUid> expected_63 = {AgentUid(42), AgentUid(43), AgentUid(46),
                                       AgentUid(47), AgentUid(58), AgentUid(59),
                                       AgentUid(62)};

  std::sort(neighbors[AgentUid(0)].begin(), neighbors[AgentUid(0)].end());
  std::sort(neighbors[AgentUid(4)].begin(), neighbors[AgentUid(4)].end());
  std::sort(neighbors[AgentUid(42)].begin(), neighbors[AgentUid(42)].end());
  std::sort(neighbors[AgentUid(63)].begin(), neighbors[AgentUid(63)].end());

  EXPECT_EQ(expected_0, neighbors[AgentUid(0)]);
  EXPECT_EQ(expected_4, neighbors[AgentUid(4)]);
  EXPECT_EQ(expected_42, neighbors[AgentUid(42)]);
  EXPECT_EQ(expected_63, neighbors[AgentUid(63)]);
}

// Test if SetEnvironment method works correctly for KDTreeEnvironment.
TEST(KDTreeTest, SetEnvironment) {
  Simulation simulation(TEST_NAME);
  auto* env = new KDTreeEnvironment();
  EXPECT_NE(env, simulation.GetEnvironment());
  simulation.SetEnvironment(env);
  EXPECT_EQ(env, simulation.GetEnvironment());
}

// Tests if ForEachNeighbor of the respective environment finds the correct
// number of neighbors. The same test is implemented for octree and unifrom grid
// environments.
TEST(KDTreeTest, FindAllNeighbors) {
  // Create simulation with kd_tree environment
  auto set_param = [](auto* param) {
    param->environment = "kd_tree";
    param->unschedule_default_operations = {"load balancing",
                                            "mechanical forces"};
  };
  Simulation simulation(TEST_NAME, set_param);

  // Please consult the definition of the fuction for more information.
  TestNeighborSearch(simulation);
}

}  // namespace bdm