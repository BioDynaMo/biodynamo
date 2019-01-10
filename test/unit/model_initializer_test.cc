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

#include "model_initializer.h"
#include "backend.h"
#include "biology_module_util.h"
#include "cell.h"
#include "compile_time_list.h"
#include "gtest/gtest.h"
#include "resource_manager.h"
#include "simulation_implementation.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"

namespace bdm {
namespace model_initializer_test_internal {

// Tests if pos_0 cubic 3D grid of cells is correctly initialized
TEST(ModelInitializerTest, Grid3DCube) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  ModelInitializer::Grid3D(2, 12, [](const std::array<double, 3>& pos) {
    Cell cell(pos);
    return cell;
  });

  const auto* cells = rm->Get<Cell>();
  EXPECT_EQ(8u, cells->size());
  EXPECT_ARR_EQ({0, 0, 0}, (*cells)[0].GetPosition());
  EXPECT_ARR_EQ({0, 0, 12}, (*cells)[1].GetPosition());
  EXPECT_ARR_EQ({0, 12, 0}, (*cells)[2].GetPosition());
  EXPECT_ARR_EQ({0, 12, 12}, (*cells)[3].GetPosition());
  EXPECT_ARR_EQ({12, 0, 0}, (*cells)[4].GetPosition());
  EXPECT_ARR_EQ({12, 0, 12}, (*cells)[5].GetPosition());
  EXPECT_ARR_EQ({12, 12, 0}, (*cells)[6].GetPosition());
  EXPECT_ARR_EQ({12, 12, 12}, (*cells)[7].GetPosition());
}

// Tests if pos_0 cuboid 3D grid of cells is correctly initialized
TEST(ModelInitializerTest, Grid3DCuboid) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  std::array<size_t, 3> grid_dimensions = {2, 3, 4};

  ModelInitializer::Grid3D(grid_dimensions, 12,
                           [](const std::array<double, 3>& pos) {
                             Cell cell(pos);
                             return cell;
                           });

  const auto* cells = rm->Get<Cell>();
  EXPECT_EQ(24u, cells->size());
  EXPECT_ARR_EQ({0, 0, 0}, (*cells)[0].GetPosition());
  EXPECT_ARR_EQ({0, 0, 12}, (*cells)[1].GetPosition());
  EXPECT_ARR_EQ({0, 0, 24}, (*cells)[2].GetPosition());
  EXPECT_ARR_EQ({0, 0, 36}, (*cells)[3].GetPosition());
  EXPECT_ARR_EQ({0, 12, 0}, (*cells)[4].GetPosition());
  EXPECT_ARR_EQ({0, 12, 12}, (*cells)[5].GetPosition());
  EXPECT_ARR_EQ({0, 12, 24}, (*cells)[6].GetPosition());
  EXPECT_ARR_EQ({12, 24, 36}, (*cells)[23].GetPosition());
}

TEST(ModelInitializerTest, CreateCells) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  std::vector<std::array<double, 3>> positions;
  positions.push_back({1, 2, 3});
  positions.push_back({101, 202, 303});
  positions.push_back({-12, -32, 4});

  ModelInitializer::CreateCells(positions,
                                [](const std::array<double, 3>& pos) {
                                  Cell cell(pos);
                                  return cell;
                                });

  const auto* cells = rm->Get<Cell>();
  EXPECT_EQ(3u, cells->size());
  EXPECT_ARR_EQ({1, 2, 3}, (*cells)[0].GetPosition());
  EXPECT_ARR_EQ({101, 202, 303}, (*cells)[1].GetPosition());
  EXPECT_ARR_EQ({-12, -32, 4}, (*cells)[2].GetPosition());
}

TEST(ModelInitializerTest, CreateCellsRandom) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();

  ModelInitializer::CreateCellsRandom(-100, 100, 10,
                                      [](const std::array<double, 3>& pos) {
                                        Cell cell(pos);
                                        return cell;
                                      });
  const auto* cells = rm->Get<Cell>();
  EXPECT_EQ(10u, cells->size());
  auto& pos_0 = (*cells)[0].GetPosition();
  auto& pos_1 = (*cells)[1].GetPosition();
  auto& pos_2 = (*cells)[2].GetPosition();
  EXPECT_TRUE((pos_0[0] >= -100) && (pos_0[0] <= 100));
  EXPECT_TRUE((pos_0[1] >= -100) && (pos_0[1] <= 100));
  EXPECT_TRUE((pos_0[2] >= -100) && (pos_0[2] <= 100));

  EXPECT_TRUE((pos_1[0] >= -100) && (pos_1[0] <= 100));
  EXPECT_TRUE((pos_1[1] >= -100) && (pos_1[1] <= 100));
  EXPECT_TRUE((pos_1[2] >= -100) && (pos_1[2] <= 100));

  EXPECT_TRUE((pos_2[0] >= -100) && (pos_2[0] <= 100));
  EXPECT_TRUE((pos_2[1] >= -100) && (pos_2[1] <= 100));
  EXPECT_TRUE((pos_2[2] >= -100) && (pos_2[2] <= 100));
}

}  // namespace model_initializer_test_internal
}  // namespace bdm
