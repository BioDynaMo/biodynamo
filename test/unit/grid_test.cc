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

#include "grid.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"

namespace bdm {

template <typename TContainer>
void CellFactory(TContainer* cells, size_t cells_per_dim) {
  const double space = 20;
  cells->reserve(cells_per_dim * cells_per_dim * cells_per_dim);
  for (size_t i = 0; i < cells_per_dim; i++) {
    for (size_t j = 0; j < cells_per_dim; j++) {
      for (size_t k = 0; k < cells_per_dim; k++) {
        Cell cell({k * space, j * space, i * space});
        cell.SetDiameter(30);
        cells->push_back(cell);
      }
    }
  }
}

TEST(GridTest, SetupGrid) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();
  CellFactory(cells, 4);

  auto& grid = Grid<>::GetInstance();
  grid.Initialize();

  std::vector<std::vector<SoHandle>> neighbors(cells->size());

// Lambda that fills a vector of neighbors for each cell (excluding itself)
#pragma omp parallel for
  for (size_t i = 0; i < cells->size(); i++) {
    auto&& cell = (*cells)[i];
    auto fill_neighbor_list = [&](auto&& neighbor, SoHandle handle) {
      if (i != handle.GetElementIdx()) {
        neighbors[i].push_back(handle);
      }
    };

    grid.ForEachNeighborWithinRadius(fill_neighbor_list, cell, SoHandle(0, i),
                                     1201);
  }

  std::vector<SoHandle> expected_0 = {
      SoHandle(0, 1),  SoHandle(0, 4),  SoHandle(0, 5), SoHandle(0, 16),
      SoHandle(0, 17), SoHandle(0, 20), SoHandle(0, 21)};
  std::vector<SoHandle> expected_4 = {
      SoHandle(0, 0),  SoHandle(0, 1),  SoHandle(0, 5),  SoHandle(0, 8),
      SoHandle(0, 9),  SoHandle(0, 16), SoHandle(0, 17), SoHandle(0, 20),
      SoHandle(0, 21), SoHandle(0, 24), SoHandle(0, 25)};
  std::vector<SoHandle> expected_42 = {
      SoHandle(0, 21), SoHandle(0, 22), SoHandle(0, 23), SoHandle(0, 25),
      SoHandle(0, 26), SoHandle(0, 27), SoHandle(0, 29), SoHandle(0, 30),
      SoHandle(0, 31), SoHandle(0, 37), SoHandle(0, 38), SoHandle(0, 39),
      SoHandle(0, 41), SoHandle(0, 43), SoHandle(0, 45), SoHandle(0, 46),
      SoHandle(0, 47), SoHandle(0, 53), SoHandle(0, 54), SoHandle(0, 55),
      SoHandle(0, 57), SoHandle(0, 58), SoHandle(0, 59), SoHandle(0, 61),
      SoHandle(0, 62), SoHandle(0, 63)};
  std::vector<SoHandle> expected_63 = {
      SoHandle(0, 42), SoHandle(0, 43), SoHandle(0, 46), SoHandle(0, 47),
      SoHandle(0, 58), SoHandle(0, 59), SoHandle(0, 62)};

  std::sort(neighbors[0].begin(), neighbors[0].end());
  std::sort(neighbors[4].begin(), neighbors[4].end());
  std::sort(neighbors[42].begin(), neighbors[42].end());
  std::sort(neighbors[63].begin(), neighbors[63].end());

  EXPECT_EQ(expected_0, neighbors[0]);
  EXPECT_EQ(expected_4, neighbors[4]);
  EXPECT_EQ(expected_42, neighbors[42]);
  EXPECT_EQ(expected_63, neighbors[63]);
}

template <typename TContainer>
void RunUpdateGridTest(TContainer* cells) {
  auto& grid = Grid<>::GetInstance();

  // Update the grid
  grid.UpdateGrid();

  std::vector<std::vector<SoHandle>> neighbors(cells->size());

// Lambda that fills a vector of neighbors for each cell (excluding itself)
#pragma omp parallel for
  for (size_t i = 0; i < cells->size(); i++) {
    auto&& cell = (*cells)[i];
    auto fill_neighbor_list = [&](auto&& neighbor, SoHandle handle) {
      if (i != handle.GetElementIdx()) {
        neighbors[i].push_back(handle);
      }
    };

    grid.ForEachNeighborWithinRadius(fill_neighbor_list, cell, SoHandle(0, i),
                                     1201);
  }

  std::vector<SoHandle> expected_0 = {SoHandle(0, 4),  SoHandle(0, 5),
                                      SoHandle(0, 16), SoHandle(0, 17),
                                      SoHandle(0, 20), SoHandle(0, 21)};
  std::vector<SoHandle> expected_5 = {
      SoHandle(0, 0),  SoHandle(0, 2),  SoHandle(0, 4),  SoHandle(0, 6),
      SoHandle(0, 8),  SoHandle(0, 9),  SoHandle(0, 10), SoHandle(0, 16),
      SoHandle(0, 17), SoHandle(0, 18), SoHandle(0, 20), SoHandle(0, 21),
      SoHandle(0, 22), SoHandle(0, 24), SoHandle(0, 25), SoHandle(0, 26)};
  std::vector<SoHandle> expected_41 = {
      SoHandle(0, 1),  SoHandle(0, 20), SoHandle(0, 21), SoHandle(0, 22),
      SoHandle(0, 24), SoHandle(0, 25), SoHandle(0, 26), SoHandle(0, 28),
      SoHandle(0, 29), SoHandle(0, 30), SoHandle(0, 36), SoHandle(0, 37),
      SoHandle(0, 38), SoHandle(0, 40), SoHandle(0, 44), SoHandle(0, 45),
      SoHandle(0, 46), SoHandle(0, 52), SoHandle(0, 53), SoHandle(0, 54),
      SoHandle(0, 56), SoHandle(0, 57), SoHandle(0, 58), SoHandle(0, 60),
      SoHandle(0, 61)};
  std::vector<SoHandle> expected_61 = {
      SoHandle(0, 1),  SoHandle(0, 40), SoHandle(0, 41), SoHandle(0, 44),
      SoHandle(0, 45), SoHandle(0, 46), SoHandle(0, 56), SoHandle(0, 57),
      SoHandle(0, 58), SoHandle(0, 60)};

  std::sort(neighbors[0].begin(), neighbors[0].end());
  std::sort(neighbors[5].begin(), neighbors[5].end());
  std::sort(neighbors[41].begin(), neighbors[41].end());
  std::sort(neighbors[61].begin(), neighbors[61].end());

  EXPECT_EQ(expected_0, neighbors[0]);
  EXPECT_EQ(expected_5, neighbors[5]);
  EXPECT_EQ(expected_41, neighbors[41]);
  EXPECT_EQ(expected_61, neighbors[61]);
}

TEST(GridTest, UpdateGrid) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();
  CellFactory(cells, 4);

  auto& grid = Grid<>::GetInstance();
  grid.Initialize();

  // Remove cells 1 and 42 (they are swapped with the last two cells)
  cells->DelayedRemove(1);
  cells->DelayedRemove(42);
  cells->Commit();

  EXPECT_EQ(62u, cells->size());

  RunUpdateGridTest(cells);
}

TEST(GridTest, NoRaceConditionDuringUpdate) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();
  CellFactory(cells, 4);

  // make sure that there are multiple cells per box
  (*cells)[0].SetDiameter(60);

  auto& grid = Grid<>::GetInstance();
  grid.Initialize();

  // Remove cells 1 and 42 (they are swapped with the last two cells)
  cells->DelayedRemove(1);
  cells->DelayedRemove(42);
  cells->Commit();

  // run 100 times to increase possibility of race condition due to different
  // scheduling of threads
  for (uint16_t i = 0; i < 100; i++) {
    RunUpdateGridTest(cells);
  }
}

TEST(GridTest, GetBoxIndex) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();
  CellFactory(cells, 3);

  auto& grid = Grid<>::GetInstance();
  grid.Initialize();

  std::array<double, 3> position_0 = {{0, 0, 0}};
  std::array<double, 3> position_1 = {{1e-15, 1e-15, 1e-15}};
  std::array<double, 3> position_2 = {{-1e-15, 1e-15, 1e-15}};

  size_t expected_idx_0 = 21;
  size_t expected_idx_1 = 21;
  size_t expected_idx_2 = 20;

  size_t idx_0 = grid.GetBoxIndex(position_0);
  size_t idx_1 = grid.GetBoxIndex(position_1);
  size_t idx_2 = grid.GetBoxIndex(position_2);

  EXPECT_EQ(expected_idx_0, idx_0);
  EXPECT_EQ(expected_idx_1, idx_1);
  EXPECT_EQ(expected_idx_2, idx_2);
}

TEST(GridTest, GridDimensions) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();
  CellFactory(cells, 3);

  auto& grid = Grid<>::GetInstance();
  grid.Initialize();

  std::array<int32_t, 6> expected_dim_0 = {{-30, 90, -30, 90, -30, 90}};
  auto& dim_0 = grid.GetDimensions();

  EXPECT_EQ(expected_dim_0, dim_0);

  ((*cells)[0]).SetPosition({{100, 0, 0}});
  grid.UpdateGrid();
  std::array<int32_t, 6> expected_dim_1 = {{-30, 150, -30, 90, -30, 90}};
  auto& dim_1 = grid.GetDimensions();

  EXPECT_EQ(expected_dim_1, dim_1);
}

TEST(GridTest, GetBoxCoordinates) {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();
  CellFactory(cells, 3);

  // expecting a 4 * 4 * 4 grid
  auto& grid = Grid<>::GetInstance();
  grid.Initialize();

  EXPECT_ARR_EQ({3, 0, 0}, grid.GetBoxCoordinates(3));
  EXPECT_ARR_EQ({1, 2, 0}, grid.GetBoxCoordinates(9));
  EXPECT_ARR_EQ({1, 2, 3}, grid.GetBoxCoordinates(57));
}

void RunNoRaceConditionForEachPairTest() {
  auto rm = ResourceManager<>::Get();
  rm->Clear();
  auto cells = rm->Get<Cell>();
  CellFactory(cells, 3);

  // expecting a 4 * 4 * 4 grid
  auto& grid = Grid<>::GetInstance();
  grid.Initialize();

  std::vector<int> result(cells->size());

  auto lambda = [&](auto&& lhs, SoHandle lhs_id, auto&& rhs, SoHandle rhs_id) {
    result[lhs_id.GetElementIdx()]++;
    result[rhs_id.GetElementIdx()]++;
  };

  // space between cells is 20 -> 20^2 + 1 = 401
  grid.ForEachNeighborPairWithinRadius(lambda, 401);

  EXPECT_EQ(3, result[0]);
  EXPECT_EQ(4, result[1]);
  EXPECT_EQ(3, result[2]);
  EXPECT_EQ(4, result[3]);
  EXPECT_EQ(5, result[4]);
  EXPECT_EQ(4, result[5]);
  EXPECT_EQ(3, result[6]);
  EXPECT_EQ(4, result[7]);
  EXPECT_EQ(3, result[8]);
  EXPECT_EQ(4, result[9]);
  EXPECT_EQ(5, result[10]);
  EXPECT_EQ(4, result[11]);
  EXPECT_EQ(5, result[12]);
  EXPECT_EQ(6, result[13]);
  EXPECT_EQ(5, result[14]);
  EXPECT_EQ(4, result[15]);
  EXPECT_EQ(5, result[16]);
  EXPECT_EQ(4, result[17]);
  EXPECT_EQ(3, result[18]);
  EXPECT_EQ(4, result[19]);
  EXPECT_EQ(3, result[20]);
  EXPECT_EQ(4, result[21]);
  EXPECT_EQ(5, result[22]);
  EXPECT_EQ(4, result[23]);
  EXPECT_EQ(3, result[24]);
  EXPECT_EQ(4, result[25]);
  EXPECT_EQ(3, result[26]);
}

TEST(GridTest, NoRaceConditionForEachPair) {
  for (int i = 0; i < 10; i++) {
    RunNoRaceConditionForEachPairTest();
  }
}

// TODO(lukas) test with different kind of cells

}  // namespace bdm
