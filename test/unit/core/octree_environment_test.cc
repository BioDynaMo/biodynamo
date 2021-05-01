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

#include "core/environment/octree_environment.h"
#include "core/agent/cell.h"
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
  rm->ForEachAgent([&](Agent* so) {
    auto uid = so->GetUid();
    FillNeighborList fill_neighbor_list(&neighbors, uid);
    grid->ForEachNeighbor(fill_neighbor_list, *so, 1201);
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

}  // namespace bdm