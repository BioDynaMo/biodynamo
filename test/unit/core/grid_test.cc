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

#include "core/grid.h"
#include "core/sim_object/cell.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {

void CellFactory(ResourceManager* rm, size_t cells_per_dim) {
  const double space = 20;
  rm->Reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell* cell = new Cell({k * space, j * space, i * space});
        cell->SetDiameter(30);
        rm->push_back(cell);
      }
    }
  }
}

TEST(GridTest, SetupGrid) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto ref_uid = SoUidGenerator::Get()->GetLastId();

  CellFactory(rm, 4);

  grid->Initialize();

  std::unordered_map<SoUid, std::vector<SoUid>> neighbors;
  neighbors.reserve(rm->GetNumSimObjects());

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  rm->ApplyOnAllElements([&](SimObject* so) {
    auto uid = so->GetUid();
    auto fill_neighbor_list = [&](const SimObject* neighbor) {
      auto nuid = neighbor->GetUid();
      if (uid != nuid) {
        neighbors[uid].push_back(nuid);
      }
    };

    grid->ForEachNeighborWithinRadius(fill_neighbor_list, *so, 1201);
  });

  std::vector<SoUid> expected_0 = { 1, 4, 5, 16, 17, 20, 21 };
  std::vector<SoUid> expected_4 = { 0, 1, 5, 8, 9, 16, 17, 20, 21, 24, 25 };
  std::vector<SoUid> expected_42 = { 21, 22, 23, 25, 26, 27, 29, 30, 31, 37, 38, 39, 41, 43, 45, 46, 47, 53, 54, 55, 57, 58, 59, 61, 62, 63 };
  std::vector<SoUid> expected_63 = { 42, 43, 46, 47, 58, 59, 62 };

  for(auto& el : expected_0) { el += ref_uid; }
  for(auto& el : expected_4) { el += ref_uid; }
  for(auto& el : expected_42) { el += ref_uid; }
  for(auto& el : expected_63) { el += ref_uid; }

  std::sort(neighbors[ref_uid].begin(), neighbors[ref_uid].end());
  std::sort(neighbors[ref_uid + 4].begin(), neighbors[ref_uid + 4].end());
  std::sort(neighbors[ref_uid + 42].begin(), neighbors[ref_uid + 42].end());
  std::sort(neighbors[ref_uid + 63].begin(), neighbors[ref_uid + 63].end());

  EXPECT_EQ(expected_0, neighbors[ref_uid]);
  EXPECT_EQ(expected_4, neighbors[ref_uid + 4]);
  EXPECT_EQ(expected_42, neighbors[ref_uid + 42]);
  EXPECT_EQ(expected_63, neighbors[ref_uid + 63]);
}

void RunUpdateGridTest(Simulation* simulation, SoUid ref_uid) {
  auto* rm = simulation->GetResourceManager();
  auto* grid = simulation->GetGrid();

  // Update the grid
  grid->UpdateGrid();

  std::unordered_map<SoUid, std::vector<SoUid>> neighbors;
  neighbors.reserve(rm->GetNumSimObjects());

  // Lambda that fills a vector of neighbors for each cell (excluding itself)
  rm->ApplyOnAllElements([&](SimObject* so) {
    auto uid = so->GetUid();
    auto fill_neighbor_list = [&](const SimObject* neighbor) {
      auto nuid = neighbor->GetUid();
      if (uid != nuid) {
        neighbors[uid].push_back(nuid);
      }
    };

    grid->ForEachNeighborWithinRadius(fill_neighbor_list, *so, 1201);
  });

  std::vector<SoUid> expected_0 = { 4, 5, 16, 17, 20, 21 };
  std::vector<SoUid> expected_5 = { 0, 2, 4, 6, 8, 9, 10, 16, 17, 18, 20, 21, 22, 24, 25, 26 };
  std::vector<SoUid> expected_41 = { 20, 21, 22, 24, 25, 26, 28, 29, 30, 36, 37,
    38, 40, 44, 45, 46, 52, 53, 54, 56, 57, 58, 60, 61, 62 };
  std::vector<SoUid> expected_61 = { 40, 41, 44, 45, 46 ,56, 57, 58, 60, 62};

  for(auto& el : expected_0) { el += ref_uid; }
  for(auto& el : expected_5) { el += ref_uid; }
  for(auto& el : expected_41) { el += ref_uid; }
  for(auto& el : expected_61) { el += ref_uid; }

  std::sort(neighbors[ref_uid].begin(), neighbors[ref_uid].end());
  std::sort(neighbors[ref_uid + 5].begin(), neighbors[ref_uid + 5].end());
  std::sort(neighbors[ref_uid + 41].begin(), neighbors[ref_uid + 41].end());
  std::sort(neighbors[ref_uid + 61].begin(), neighbors[ref_uid + 61].end());

  EXPECT_EQ(expected_0, neighbors[ref_uid]);
  EXPECT_EQ(expected_5, neighbors[ref_uid + 5]);
  EXPECT_EQ(expected_41, neighbors[ref_uid + 41]);
  EXPECT_EQ(expected_61, neighbors[ref_uid + 61]);
}

TEST(GridTest, UpdateGrid) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto ref_uid = SoUidGenerator::Get()->GetLastId();

  CellFactory(rm, 4);

  grid->Initialize();

  // Remove cells 1 and 42
  rm->Remove(ref_uid + 1);
  rm->Remove(ref_uid + 42);

  EXPECT_EQ(62u, rm->GetNumSimObjects());

  RunUpdateGridTest(&simulation, ref_uid);
}

TEST(GridTest, NoRaceConditionDuringUpdate) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto ref_uid = SoUidGenerator::Get()->GetLastId();

  CellFactory(rm, 4);

  // make sure that there are multiple cells per box
  rm->GetSimObject(ref_uid)->SetDiameter(60);

  grid->Initialize();

  // Remove cells 1 and 42
  rm->Remove(ref_uid + 1);
  rm->Remove(ref_uid + 42);

  // run 100 times to increase possibility of race condition due to different
  // scheduling of threads
  for (uint16_t i = 0; i < 100; i++) {
    RunUpdateGridTest(&simulation, ref_uid);
  }
}

TEST(GridTest, GetBoxIndex) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  CellFactory(rm, 3);

  grid->Initialize();

  std::array<double, 3> position_0 = {{0, 0, 0}};
  std::array<double, 3> position_1 = {{1e-15, 1e-15, 1e-15}};
  std::array<double, 3> position_2 = {{-1e-15, 1e-15, 1e-15}};

  size_t expected_idx_0 = 21;
  size_t expected_idx_1 = 21;
  size_t expected_idx_2 = 20;

  size_t idx_0 = grid->GetBoxIndex(position_0);
  size_t idx_1 = grid->GetBoxIndex(position_1);
  size_t idx_2 = grid->GetBoxIndex(position_2);

  EXPECT_EQ(expected_idx_0, idx_0);
  EXPECT_EQ(expected_idx_1, idx_1);
  EXPECT_EQ(expected_idx_2, idx_2);
}

TEST(GridTest, GridDimensions) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto ref_uid = SoUidGenerator::Get()->GetLastId();

  CellFactory(rm, 3);

  grid->Initialize();

  std::array<int32_t, 6> expected_dim_0 = {{-30, 90, -30, 90, -30, 90}};
  auto& dim_0 = grid->GetDimensions();

  EXPECT_EQ(expected_dim_0, dim_0);

  rm->GetSimObject(ref_uid)->SetPosition({{100, 0, 0}});
  grid->UpdateGrid();
  std::array<int32_t, 6> expected_dim_1 = {{-30, 150, -30, 90, -30, 90}};
  auto& dim_1 = grid->GetDimensions();

  EXPECT_EQ(expected_dim_1, dim_1);
}

TEST(GridTest, GetBoxCoordinates) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  CellFactory(rm, 3);

  // expecting a 4 * 4 * 4 grid
  grid->Initialize();

  EXPECT_ARR_EQ({3, 0, 0}, grid->GetBoxCoordinates(3));
  EXPECT_ARR_EQ({1, 2, 0}, grid->GetBoxCoordinates(9));
  EXPECT_ARR_EQ({1, 2, 3}, grid->GetBoxCoordinates(57));
}

TEST(GridTest, NonEmptyBoundedTestThresholdDimensions) {
  auto set_param = [](auto* param) {
    param->bound_space_ = true;
    param->min_bound_ = 1;
    param->max_bound_ = 99;
  };

  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  rm->push_back(new Cell(10));

  grid->Initialize();

  auto max_dimensions = grid->GetDimensionThresholds();
  EXPECT_EQ(1, max_dimensions[0]);
  EXPECT_EQ(99, max_dimensions[1]);
}

TEST(GridTest, IterateZOrder) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto ref_uid = SoUidGenerator::Get()->GetLastId();
  CellFactory(rm, 3);

  // expecting a 4 * 4 * 4 grid
  grid->Initialize();

  std::vector<SoUid> zorder;
  zorder.reserve(rm->GetNumSimObjects());
  auto lambda = [&](SimObject* so) { zorder.push_back(so->GetUid()); };
  grid->IterateZOrder(lambda);

  ASSERT_EQ(27u, zorder.size());
  // boxes separated by comments //
  EXPECT_EQ(ref_uid + 0, zorder[0]);
  EXPECT_EQ(ref_uid + 1, zorder[1]);
  EXPECT_EQ(ref_uid + 3, zorder[2]);
  EXPECT_EQ(ref_uid + 4, zorder[3]);
  EXPECT_EQ(ref_uid + 9, zorder[4]);
  EXPECT_EQ(ref_uid + 10, zorder[5]);
  EXPECT_EQ(ref_uid + 12, zorder[6]);
  EXPECT_EQ(ref_uid + 13, zorder[7]);
  //
  EXPECT_EQ(ref_uid + 2, zorder[8]);
  EXPECT_EQ(ref_uid + 5, zorder[9]);
  EXPECT_EQ(ref_uid + 11, zorder[10]);
  EXPECT_EQ(ref_uid + 14, zorder[11]);
  //
  EXPECT_EQ(ref_uid + 6, zorder[12]);
  EXPECT_EQ(ref_uid + 7, zorder[13]);
  EXPECT_EQ(ref_uid + 15, zorder[14]);
  EXPECT_EQ(ref_uid + 16, zorder[15]);
  //
  EXPECT_EQ(ref_uid + 8, zorder[16]);
  EXPECT_EQ(ref_uid + 17, zorder[17]);
  //
  EXPECT_EQ(ref_uid + 18, zorder[18]);
  EXPECT_EQ(ref_uid + 19, zorder[19]);
  EXPECT_EQ(ref_uid + 21, zorder[20]);
  EXPECT_EQ(ref_uid + 22, zorder[21]);
  //
  EXPECT_EQ(ref_uid + 20, zorder[22]);
  EXPECT_EQ(ref_uid + 23, zorder[23]);
  //
  EXPECT_EQ(ref_uid + 24, zorder[24]);
  EXPECT_EQ(ref_uid + 25, zorder[25]);
  //
  EXPECT_EQ(ref_uid + 26, zorder[26]);
}

}  // namespace bdm
