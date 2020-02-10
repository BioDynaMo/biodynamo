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

#include "core/model_initializer.h"
#include "core/biology_module/biology_module.h"
#include "core/resource_manager.h"
#include "core/sim_object/cell.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace model_initializer_test_internal {

// Tests if pos_0 cubic 3D grid of cells is correctly initialized
TEST(ModelInitializerTest, Grid3DCube) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto ref_uid = simulation.GetSoUidGenerator()->GetLastId();

  ModelInitializer::Grid3D(2, 12, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  EXPECT_EQ(8u, rm->GetNumSimObjects());
  EXPECT_ARR_EQ({0, 0, 0}, rm->GetSimObject(ref_uid + 0)->GetPosition());
  EXPECT_ARR_EQ({0, 0, 12}, rm->GetSimObject(ref_uid + 1)->GetPosition());
  EXPECT_ARR_EQ({0, 12, 0}, rm->GetSimObject(ref_uid + 2)->GetPosition());
  EXPECT_ARR_EQ({0, 12, 12}, rm->GetSimObject(ref_uid + 3)->GetPosition());
  EXPECT_ARR_EQ({12, 0, 0}, rm->GetSimObject(ref_uid + 4)->GetPosition());
  EXPECT_ARR_EQ({12, 0, 12}, rm->GetSimObject(ref_uid + 5)->GetPosition());
  EXPECT_ARR_EQ({12, 12, 0}, rm->GetSimObject(ref_uid + 6)->GetPosition());
  EXPECT_ARR_EQ({12, 12, 12}, rm->GetSimObject(ref_uid + 7)->GetPosition());
}

// Tests if pos_0 cuboid 3D grid of cells is correctly initialized
TEST(ModelInitializerTest, Grid3DCuboid) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto ref_uid = simulation.GetSoUidGenerator()->GetLastId();

  std::array<size_t, 3> grid_dimensions = {2, 3, 4};

  ModelInitializer::Grid3D(grid_dimensions, 12, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  EXPECT_EQ(24u, rm->GetNumSimObjects());
  EXPECT_ARR_EQ({0, 0, 0}, rm->GetSimObject(ref_uid + 0)->GetPosition());
  EXPECT_ARR_EQ({0, 0, 12}, rm->GetSimObject(ref_uid + 1)->GetPosition());
  EXPECT_ARR_EQ({0, 0, 24}, rm->GetSimObject(ref_uid + 2)->GetPosition());
  EXPECT_ARR_EQ({0, 0, 36}, rm->GetSimObject(ref_uid + 3)->GetPosition());
  EXPECT_ARR_EQ({0, 12, 0}, rm->GetSimObject(ref_uid + 4)->GetPosition());
  EXPECT_ARR_EQ({0, 12, 12}, rm->GetSimObject(ref_uid + 5)->GetPosition());
  EXPECT_ARR_EQ({0, 12, 24}, rm->GetSimObject(ref_uid + 6)->GetPosition());
  EXPECT_ARR_EQ({12, 24, 36}, rm->GetSimObject(ref_uid + 23)->GetPosition());
}

TEST(ModelInitializerTest, CreateCells) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto ref_uid = simulation.GetSoUidGenerator()->GetLastId();

  std::vector<Double3> positions;
  positions.push_back({1, 2, 3});
  positions.push_back({101, 202, 303});
  positions.push_back({-12, -32, 4});

  ModelInitializer::CreateCells(positions, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });

  EXPECT_EQ(3u, rm->GetNumSimObjects());
  EXPECT_ARR_EQ({1, 2, 3}, rm->GetSimObject(ref_uid + 0)->GetPosition());
  EXPECT_ARR_EQ({101, 202, 303}, rm->GetSimObject(ref_uid + 1)->GetPosition());
  EXPECT_ARR_EQ({-12, -32, 4}, rm->GetSimObject(ref_uid + 2)->GetPosition());
}

TEST(ModelInitializerTest, CreateCellsRandom) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto ref_uid = simulation.GetSoUidGenerator()->GetLastId();

  ModelInitializer::CreateCellsRandom(-100, 100, 10, [](const Double3& pos) {
    Cell* cell = new Cell(pos);
    return cell;
  });
  EXPECT_EQ(10u, rm->GetNumSimObjects());
  auto& pos_0 = rm->GetSimObject(ref_uid + 0)->GetPosition();
  auto& pos_1 = rm->GetSimObject(ref_uid + 1)->GetPosition();
  auto& pos_2 = rm->GetSimObject(ref_uid + 2)->GetPosition();
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
