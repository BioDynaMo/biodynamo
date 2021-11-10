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

#include "core/environment/octree_environment.h"
#include "core/agent/cell.h"
#include "unit/core/count_neighbor_functor.h"
#include "unit/test_util/test_util.h"

#include "gtest/gtest.h"

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

TEST(OctreeTest, Setup) {
  auto set_param = [](auto* param) { param->environment = "octree"; };
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* grid = dynamic_cast<OctreeEnvironment*>(simulation.GetEnvironment());

  EXPECT_NE(nullptr, grid);

  CellFactory(rm, 4);

  grid->Update();

  std::unordered_map<AgentUid, std::vector<AgentUid>> neighbors;
  neighbors.reserve(rm->GetNumAgents());

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  double search_radius_squared = 1201;
  rm->ForEachAgent([&](Agent* so) {
    auto uid = so->GetUid();
    FillNeighborList fill_neighbor_list(&neighbors, uid);
    grid->ForEachNeighbor(fill_neighbor_list, *so, search_radius_squared);
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

// Test if SetEnvironment method works correctly for OctreeEnvironment.
TEST(OctreeTest, SetEnvironment) {
  Simulation simulation(TEST_NAME);
  auto* env = new OctreeEnvironment();
  EXPECT_NE(env, simulation.GetEnvironment());
  simulation.SetEnvironment(env);
  EXPECT_EQ(env, simulation.GetEnvironment());
}

// Tests if ForEachNeighbor of the respective environment finds the correct
// number of neighbors. The same test is implemented for kdtree and unifrom grid
// environments.
TEST(OctreeTest, FindAllNeighbors) {
  // Create simulation with octree environment
  auto set_param = [](auto* param) {
    param->environment = "octree";
    param->unschedule_default_operations = {"load balancing",
                                            "mechanical forces"};
  };
  Simulation simulation(TEST_NAME, set_param);

  // Add three cells at specific positions
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* cell1 = new Cell(2.0);
  auto* cell2 = new Cell(4.0);
  auto* cell3 = new Cell(2.0);
  cell1->SetPosition({0.0, 0.0, 0.});
  cell2->SetPosition({5, 0, 0});
  cell3->SetPosition({0, -2.5, 0});
  rm->AddAgent(cell1);
  rm->AddAgent(cell2);
  rm->AddAgent(cell3);
  scheduler->Simulate(1);

  // Test if there are three agents in simulation
  EXPECT_EQ(3, rm->GetNumAgents());

  // Define dummy test points to check how many neighbors they find. Distances
  // to cells 1, 2, 3 listed as (d1, d2, d3).
  Double3 test_point_1({-0.1, 0.0, 0.0});  // (0.1, 5.1, 2.502)
  Double3 test_point_2({3.5, 0.0, 0.0});   // (3.5, 1.5, 4.30116)
  Double3 test_point_3({0.0, -2.0, 0.0});  // (2, 5.28516, 0.5)
  Double3 test_point_4({0.0, -0.8, 0.0});  // (0.8, 5.0626, 1.7)
  Double3 test_point_5({-2.1, 0.0, 0.0});  // (2.1, 7.1, 3.26497)

  // Test if we find the correct number of agents. The reference solution can
  // be determined by substracting the search_radius from the bracket behind
  // the respective test point and counting the values that are strictly smaller
  // than 0. E.g.:
  // (0.1, 5.1, 2.502) - 2 =  (-1.9, 3.1, 0.502)
  // (-1.9, 3.1, 0.502) < 0 = (1 , 0 , 0) -> result = 1
  double search_radius = 2;
  EXPECT_EQ(1, GetNeighbors(test_point_1, search_radius));
  EXPECT_EQ(1, GetNeighbors(test_point_2, search_radius));
  EXPECT_EQ(1, GetNeighbors(test_point_3, search_radius));
  EXPECT_EQ(2, GetNeighbors(test_point_4, search_radius));
  EXPECT_EQ(0, GetNeighbors(test_point_5, search_radius));
}

}  // namespace bdm