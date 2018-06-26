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

#include <fstream>

#include "cell.h"
#include "diffusion_grid.h"
#include "grid.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "simulation_implementation.h"
#include "unit/default_ctparam.h"
#include "unit/test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {

template <typename TContainer>
void CellFactory(TContainer* cells,
                 const std::vector<std::array<double, 3>>& positions) {
  cells->reserve(positions.size());
  for (size_t i = 0; i < positions.size(); i++) {
    Cell cell({positions[i][0], positions[i][1], positions[i][2]});
    cell.SetDiameter(30);
    cells->push_back(cell);
  }
}

// Test if the dimensions of the diffusion grid are corresponding to the
// neighbor grid dimensions
TEST(DiffusionTest, GridDimensions) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto cells = rm->Get<Cell>();

  std::vector<std::array<double, 3>> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(cells, positions);

  DiffusionGrid* d_grid = new DiffusionGrid(0, "Kalium", 0.4, 0, 1);

  grid->Initialize();
  d_grid->Initialize(grid->GetDimensions());

  auto dims = d_grid->GetDimensions();

  EXPECT_EQ(-40, dims[0]);
  EXPECT_EQ(-40, dims[2]);
  EXPECT_EQ(-40, dims[4]);
  EXPECT_EQ(140, dims[1]);
  EXPECT_EQ(140, dims[3]);
  EXPECT_EQ(140, dims[5]);

  delete d_grid;
}

// Test if the dimension of the diffusion grid update correctly with the
// neighbor grid dimensions (we expect the diffusion grid to stay cube-shaped)
TEST(DiffusionTest, UpdateGrid) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto cells = rm->Get<Cell>();

  std::vector<std::array<double, 3>> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(cells, positions);

  DiffusionGrid* d_grid = new DiffusionGrid(0, "Kalium", 0.4, 0, 6);

  grid->Initialize();
  d_grid->Initialize(grid->GetDimensions());

  std::vector<std::array<double, 3>> positions_2;
  positions_2.push_back({-30, -10, -10});
  positions_2.push_back({90, 150, 90});
  CellFactory(cells, positions_2);

  grid->UpdateGrid();

  d_grid->Update(grid->GetDimensionThresholds());

  auto d_dims = d_grid->GetDimensions();

  EXPECT_EQ(-90, d_dims[0]);
  EXPECT_EQ(-90, d_dims[2]);
  EXPECT_EQ(-90, d_dims[4]);
  EXPECT_EQ(210, d_dims[1]);
  EXPECT_EQ(210, d_dims[3]);
  EXPECT_EQ(210, d_dims[5]);

  delete d_grid;
}

// Test if the diffusion grid does not change if the neighbor grid dimensions
// do not change
TEST(DiffusionTest, FalseUpdateGrid) {
  Simulation<> simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  auto* grid = simulation.GetGrid();

  auto cells = rm->Get<Cell>();

  std::vector<std::array<double, 3>> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(cells, positions);

  DiffusionGrid* d_grid = new DiffusionGrid(0, "Kalium", 0.4, 0);

  grid->Initialize();
  d_grid->Initialize(grid->GetDimensions());
  d_grid->Update(grid->GetDimensionThresholds());

  auto dims = d_grid->GetDimensions();

  EXPECT_EQ(-40, dims[0]);
  EXPECT_EQ(-40, dims[2]);
  EXPECT_EQ(-40, dims[4]);
  EXPECT_EQ(140, dims[1]);
  EXPECT_EQ(140, dims[3]);
  EXPECT_EQ(140, dims[5]);

  d_grid->Update(grid->GetDimensionThresholds());

  dims = d_grid->GetDimensions();

  EXPECT_EQ(-40, dims[0]);
  EXPECT_EQ(-40, dims[2]);
  EXPECT_EQ(-40, dims[4]);
  EXPECT_EQ(140, dims[1]);
  EXPECT_EQ(140, dims[3]);
  EXPECT_EQ(140, dims[5]);

  delete d_grid;
}

// Create a 5x5x5 diffusion grid, with a substance being
// added at center box 2,2,2, causing a symmetrical diffusion
TEST(DiffusionTest, LeakingEdge) {
  Simulation<> simulation(TEST_NAME);

  DiffusionGrid* d_grid = new DiffusionGrid(0, "Kalium", 0.4, 0, 5);

  int lbound = -100;
  int rbound = 100;
  d_grid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound});
  d_grid->SetConcentrationThreshold(1e15);

  for (int i = 0; i < 100; i++) {
    d_grid->IncreaseConcentrationBy({{0, 0, 0}}, 4);
    d_grid->DiffuseWithLeakingEdge();
    d_grid->CalculateGradient();
  }

  // Get concentrations and gradients after 100 time steps
  auto conc = d_grid->GetAllConcentrations();
  auto grad = d_grid->GetAllGradients();

  array<uint32_t, 3> c = {2, 2, 2};
  array<uint32_t, 3> w = {1, 2, 2};
  array<uint32_t, 3> e = {3, 2, 2};
  array<uint32_t, 3> n = {2, 1, 2};
  array<uint32_t, 3> s = {2, 3, 2};
  array<uint32_t, 3> t = {2, 2, 1};
  array<uint32_t, 3> b = {2, 2, 3};
  array<uint32_t, 3> rand1_a = {0, 0, 0};
  array<uint32_t, 3> rand1_b = {4, 4, 4};
  array<uint32_t, 3> rand2_a = {4, 4, 2};
  array<uint32_t, 3> rand2_b = {0, 0, 2};

  auto kEps = abs_error<double>::value;

  double v1 = 9.7267657389657938;
  double v2 = 3.7281869469803648;
  double v3 = 0.12493663388071227;
  double v4 = 0.32563083857294983;
  double v5 = 0.10776198271458182;

  EXPECT_NEAR(v1, conc[d_grid->GetBoxIndex(c)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(e)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(w)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(n)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(s)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(t)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(b)], kEps);
  EXPECT_NEAR(v3, conc[d_grid->GetBoxIndex(rand1_a)], kEps);
  EXPECT_NEAR(v3, conc[d_grid->GetBoxIndex(rand1_b)], kEps);
  EXPECT_NEAR(v4, conc[d_grid->GetBoxIndex(rand2_a)], kEps);
  EXPECT_NEAR(v4, conc[d_grid->GetBoxIndex(rand2_b)], kEps);

  EXPECT_NEAR(0.0, grad[3 * (d_grid->GetBoxIndex(c)) + 1], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(e)) + 0], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(w)) + 0], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(n)) + 1], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(s)) + 1], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(t)) + 2], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(b)) + 2], kEps);

  delete d_grid;
}

// Create a 5x5x5 diffusion grid, with a substance being
// added at center box 2,2,2, causing a symmetrical diffusion
TEST(DiffusionTest, ClosedEdge) {
  DiffusionGrid* d_grid = new DiffusionGrid(0, "Kalium", 0.4, 0, 5);

  int lbound = -100;
  int rbound = 100;
  d_grid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound});
  d_grid->SetConcentrationThreshold(1e15);

  for (int i = 0; i < 100; i++) {
    d_grid->IncreaseConcentrationBy({{0, 0, 0}}, 4);
    d_grid->DiffuseWithClosedEdge();
    d_grid->CalculateGradient();
  }

  // Get concentrations and gradients after 100 time steps
  auto conc = d_grid->GetAllConcentrations();
  auto grad = d_grid->GetAllGradients();

  array<uint32_t, 3> c = {2, 2, 2};
  array<uint32_t, 3> w = {1, 2, 2};
  array<uint32_t, 3> e = {3, 2, 2};
  array<uint32_t, 3> n = {2, 1, 2};
  array<uint32_t, 3> s = {2, 3, 2};
  array<uint32_t, 3> t = {2, 2, 1};
  array<uint32_t, 3> b = {2, 2, 3};
  array<uint32_t, 3> rand1_a = {0, 0, 0};
  array<uint32_t, 3> rand1_b = {4, 4, 4};
  array<uint32_t, 3> rand2_a = {4, 4, 2};
  array<uint32_t, 3> rand2_b = {0, 0, 2};

  auto kEps = abs_error<double>::value;

  double v1 = 11.717698164878922;
  double v2 = 5.7977258086605303;
  double v3 = 2.4379152740053867;
  double v4 = 2.7287519978558121;
  double v5 = 0.10218091352733083;

  EXPECT_NEAR(v1, conc[d_grid->GetBoxIndex(c)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(e)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(w)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(n)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(s)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(t)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(b)], kEps);
  EXPECT_NEAR(v3, conc[d_grid->GetBoxIndex(rand1_a)], kEps);
  EXPECT_NEAR(v3, conc[d_grid->GetBoxIndex(rand1_b)], kEps);
  EXPECT_NEAR(v4, conc[d_grid->GetBoxIndex(rand2_a)], kEps);
  EXPECT_NEAR(v4, conc[d_grid->GetBoxIndex(rand2_b)], kEps);

  EXPECT_NEAR(0.0, grad[3 * (d_grid->GetBoxIndex(c)) + 1], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(e)) + 0], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(w)) + 0], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(n)) + 1], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(s)) + 1], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(t)) + 2], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(b)) + 2], kEps);

  delete d_grid;
}

// Tests if the concentration / gradient values are correctly copied
// after the grid has grown and DiffusionGrid::CopyOldData is called
TEST(DiffusionTest, CopyOldData) {
  DiffusionGrid* d_grid = new DiffusionGrid(0, "Kalium", 0.4, 0, 5);

  int lbound = -100;
  int rbound = 100;
  d_grid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound});
  d_grid->SetConcentrationThreshold(1e15);

  for (int i = 0; i < 100; i++) {
    d_grid->IncreaseConcentrationBy({{0, 0, 0}}, 4);
    d_grid->DiffuseWithLeakingEdge();
    d_grid->CalculateGradient();
  }

  int n_lbound = -140;
  int n_rbound = 140;
  d_grid->Update({n_lbound, n_rbound});

  // Get concentrations and gradients after 100 time steps
  auto conc = d_grid->GetAllConcentrations();
  auto grad = d_grid->GetAllGradients();

  array<uint32_t, 3> c = {3, 3, 3};
  array<uint32_t, 3> w = {2, 3, 3};
  array<uint32_t, 3> e = {4, 3, 3};
  array<uint32_t, 3> n = {3, 2, 3};
  array<uint32_t, 3> s = {3, 4, 3};
  array<uint32_t, 3> t = {3, 3, 2};
  array<uint32_t, 3> b = {3, 3, 4};
  array<uint32_t, 3> rand1_a = {1, 1, 1};
  array<uint32_t, 3> rand1_b = {5, 5, 5};
  array<uint32_t, 3> rand2_a = {5, 5, 3};
  array<uint32_t, 3> rand2_b = {1, 1, 3};

  auto kEps = abs_error<double>::value;

  double v1 = 9.7267657389657938;
  double v2 = 3.7281869469803648;
  double v3 = 0.12493663388071227;
  double v4 = 0.32563083857294983;
  double v5 = 0.10776198271458182;

  EXPECT_NEAR(v1, conc[d_grid->GetBoxIndex(c)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(e)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(w)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(n)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(s)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(t)], kEps);
  EXPECT_NEAR(v2, conc[d_grid->GetBoxIndex(b)], kEps);
  EXPECT_NEAR(v3, conc[d_grid->GetBoxIndex(rand1_a)], kEps);
  EXPECT_NEAR(v3, conc[d_grid->GetBoxIndex(rand1_b)], kEps);
  EXPECT_NEAR(v4, conc[d_grid->GetBoxIndex(rand2_a)], kEps);
  EXPECT_NEAR(v4, conc[d_grid->GetBoxIndex(rand2_b)], kEps);

  EXPECT_NEAR(0.0, grad[3 * (d_grid->GetBoxIndex(c)) + 1], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(e)) + 0], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(w)) + 0], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(n)) + 1], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(s)) + 1], kEps);
  EXPECT_NEAR(v5, grad[3 * (d_grid->GetBoxIndex(t)) + 2], kEps);
  EXPECT_NEAR(-v5, grad[3 * (d_grid->GetBoxIndex(b)) + 2], kEps);

  delete d_grid;
}

// Test if all the data members of the diffusion grid are correctly serialized
// and deserialzed with I/O
TEST(DiffusionTest, IOTest) {
  remove(ROOTFILE);

  DiffusionGrid* d_grid = new DiffusionGrid(0, "Kalium", 0.6, 0);

  // Create a 100x100x100 diffusion grid with 20 boxes per dimension
  std::array<int32_t, 6> dimensions = {{-50, 50, -50, 50, -50, 50}};
  d_grid->Initialize(dimensions);
  d_grid->SetConcentrationThreshold(42);
  d_grid->SetDecayConstant(0.01);

  // write to root file
  WritePersistentObject(ROOTFILE, "dgrid", *d_grid, "new");

  // read back
  DiffusionGrid* restored_d_grid = nullptr;
  GetPersistentObject(ROOTFILE, "dgrid", restored_d_grid);

  auto kEps = abs_error<double>::value;

  EXPECT_EQ("Kalium", restored_d_grid->GetSubstanceName());
  EXPECT_EQ(10, restored_d_grid->GetBoxLength());
  EXPECT_EQ(42, restored_d_grid->GetConcentrationThreshold());
  EXPECT_NEAR(0.4, restored_d_grid->GetDiffusionCoefficients()[0], kEps);
  EXPECT_NEAR(0.1, restored_d_grid->GetDiffusionCoefficients()[1], kEps);
  EXPECT_NEAR(0.1, restored_d_grid->GetDiffusionCoefficients()[2], kEps);
  EXPECT_NEAR(0.1, restored_d_grid->GetDiffusionCoefficients()[3], kEps);
  EXPECT_NEAR(0.1, restored_d_grid->GetDiffusionCoefficients()[4], kEps);
  EXPECT_NEAR(0.1, restored_d_grid->GetDiffusionCoefficients()[5], kEps);
  EXPECT_NEAR(0.1, restored_d_grid->GetDiffusionCoefficients()[6], kEps);
  EXPECT_NEAR(0.01, restored_d_grid->GetDecayConstant(), kEps);
  EXPECT_EQ(-50, restored_d_grid->GetDimensions()[0]);
  EXPECT_EQ(-50, restored_d_grid->GetDimensions()[2]);
  EXPECT_EQ(-50, restored_d_grid->GetDimensions()[4]);
  EXPECT_EQ(50, restored_d_grid->GetDimensions()[1]);
  EXPECT_EQ(50, restored_d_grid->GetDimensions()[3]);
  EXPECT_EQ(50, restored_d_grid->GetDimensions()[5]);
  EXPECT_EQ(10u, restored_d_grid->GetNumBoxesArray()[0]);
  EXPECT_EQ(10u, restored_d_grid->GetNumBoxesArray()[1]);
  EXPECT_EQ(10u, restored_d_grid->GetNumBoxesArray()[2]);
  EXPECT_EQ(1000u, restored_d_grid->GetNumBoxes());
  EXPECT_EQ(true, restored_d_grid->IsInitialized());
  EXPECT_EQ(10, restored_d_grid->GetResolution());

  remove(ROOTFILE);
  delete d_grid;
}

std::array<double, 3> GetRealCoordinates(const std::array<uint32_t, 3>& bc1,
                                         const std::array<uint32_t, 3>& bc2,
                                         double bl) {
  std::array<double, 3> ret;
  ret[0] = bl * (bc2[0] - bc1[0]);
  ret[1] = bl * (bc2[1] - bc1[1]);
  ret[2] = bl * (bc2[2] - bc1[2]);
  return ret;
}

double CalculateAnalyticalSolution(double init, double x, double y, double z,
                                   double diff_coef, double t) {
  return (init / pow(4 * Math::kPi * diff_coef * t, 1.5)) *
         exp(-(pow(x, 2)) / (4 * diff_coef * t) -
             (pow(y, 2)) / (4 * diff_coef * t) -
             (pow(z, 2)) / (4 * diff_coef * t));
}

TEST(DiffusionTest, Convergence) {
  double diff_coef = 0.5;
  DiffusionGrid* d_grid2 = new DiffusionGrid(0, "Kalium1", diff_coef, 0, 20);
  DiffusionGrid* d_grid4 = new DiffusionGrid(1, "Kalium4", diff_coef, 0, 40);
  DiffusionGrid* d_grid8 = new DiffusionGrid(2, "Kalium8", diff_coef, 0, 80);

  int l = -100;
  int r = 100;
  d_grid2->Initialize({l, r, l, r, l, r});
  d_grid4->Initialize({l, r, l, r, l, r});
  d_grid8->Initialize({l, r, l, r, l, r});

  d_grid2->SetConcentrationThreshold(1e15);
  d_grid4->SetConcentrationThreshold(1e15);
  d_grid8->SetConcentrationThreshold(1e15);

  // instantaneous point source
  int init = 1e5;
  std::array<double, 3> source = {{0, 0, 0}};
  d_grid2->IncreaseConcentrationBy(source,
                                   init / pow(d_grid2->GetBoxLength(), 3));
  d_grid4->IncreaseConcentrationBy(source,
                                   init / pow(d_grid4->GetBoxLength(), 3));
  d_grid8->IncreaseConcentrationBy(source,
                                   init / pow(d_grid8->GetBoxLength(), 3));

  auto conc2 = d_grid2->GetAllConcentrations();
  auto conc4 = d_grid4->GetAllConcentrations();
  auto conc8 = d_grid8->GetAllConcentrations();

  array<double, 3> marker = {10.0, 10.0, 10.0};

  int tot = 100;
  for (int t = 0; t < tot; t++) {
    d_grid2->DiffuseEuler();
    d_grid4->DiffuseEuler();
    d_grid8->DiffuseEuler();
  }

  auto rc2 = GetRealCoordinates(d_grid2->GetBoxCoordinates(source),
                                d_grid2->GetBoxCoordinates(marker),
                                d_grid2->GetBoxLength());
  auto rc4 = GetRealCoordinates(d_grid4->GetBoxCoordinates(source),
                                d_grid4->GetBoxCoordinates(marker),
                                d_grid4->GetBoxLength());
  auto rc8 = GetRealCoordinates(d_grid8->GetBoxCoordinates(source),
                                d_grid8->GetBoxCoordinates(marker),
                                d_grid8->GetBoxLength());

  auto real_val2 =
      CalculateAnalyticalSolution(init, rc2[0], rc2[1], rc2[2], diff_coef, tot);
  auto real_val4 =
      CalculateAnalyticalSolution(init, rc4[0], rc4[1], rc4[2], diff_coef, tot);
  auto real_val8 =
      CalculateAnalyticalSolution(init, rc8[0], rc8[1], rc8[2], diff_coef, tot);

  auto error2 = std::abs(real_val2 - conc2[d_grid2->GetBoxIndex(marker)]);
  auto error4 = std::abs(real_val4 - conc4[d_grid4->GetBoxIndex(marker)]);
  auto error8 = std::abs(real_val8 - conc8[d_grid8->GetBoxIndex(marker)]);

  EXPECT_TRUE(error4 < error2);
  EXPECT_TRUE(error8 < error4);
  EXPECT_NEAR(real_val8, conc8[d_grid8->GetBoxIndex(marker)], 0.02);

  delete d_grid2;
  delete d_grid4;
  delete d_grid8;
}

}  // namespace bdm
