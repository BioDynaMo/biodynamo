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

#include <fstream>

#include "core/agent/cell.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/diffusion/euler_grid.h"
#include "core/diffusion/runga_kutta_grid.h"
#include "core/diffusion/stencil_grid.h"
#include "core/environment/environment.h"
#include "core/model_initializer.h"
#include "core/substance_initializers.h"
#include "core/util/io.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

#ifdef USE_PARAVIEW
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkXMLImageDataReader.h>
#include "core/visualization/paraview/adaptor.h"
#endif  // USE_PARAVIEW

#define ROOTFILE "bdmFile.root"

namespace bdm {

void CellFactory(const std::vector<Double3>& positions) {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->Reserve(positions.size());
  for (size_t i = 0; i < positions.size(); i++) {
    Cell* cell = new Cell({positions[i][0], positions[i][1], positions[i][2]});
    cell->SetDiameter(30);
    rm->AddAgent(cell);
  }
}

// Test if the dimensions of the diffusion grid are corresponding to the
// neighbor env dimensions
TEST(DiffusionTest, GridDimensions) {
  Simulation simulation(TEST_NAME);
  auto* env = simulation.GetEnvironment();

  std::vector<Double3> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(positions);

  DiffusionGrid* dgrid = new StencilGrid(0, "Kalium", 0.4, 0, 2);

  env->Update();
  dgrid->Initialize();

  auto dims = dgrid->GetDimensions();

  EXPECT_EQ(-40, dims[0]);
  EXPECT_EQ(-40, dims[2]);
  EXPECT_EQ(-40, dims[4]);
  EXPECT_EQ(140, dims[1]);
  EXPECT_EQ(140, dims[3]);
  EXPECT_EQ(140, dims[5]);

  delete dgrid;
}

// Test if the dimension of the diffusion grid update correctly with the
// neighbor env dimensions (we expect the diffusion grid to stay cube-shaped)
TEST(DiffusionTest, UpdateGrid) {
  Simulation simulation(TEST_NAME);
  auto* env = simulation.GetEnvironment();

  std::vector<Double3> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(positions);

  DiffusionGrid* dgrid = new StencilGrid(0, "Kalium", 0.4, 0, 7);

  env->Update();
  dgrid->Initialize();

  std::vector<Double3> positions_2;
  positions_2.push_back({-30, -10, -10});
  positions_2.push_back({90, 150, 90});
  CellFactory(positions_2);

  env->Update();

  dgrid->Update();

  auto d_dims = dgrid->GetDimensions();

  EXPECT_EQ(-60, d_dims[0]);
  EXPECT_EQ(-60, d_dims[2]);
  EXPECT_EQ(-60, d_dims[4]);
  EXPECT_EQ(210, d_dims[1]);
  EXPECT_EQ(210, d_dims[3]);
  EXPECT_EQ(210, d_dims[5]);

  delete dgrid;
}

// Test if the diffusion grid does not change if the neighbor env dimensions
// do not change
TEST(DiffusionTest, FalseUpdateGrid) {
  Simulation simulation(TEST_NAME);
  auto* env = simulation.GetEnvironment();

  std::vector<Double3> positions;
  positions.push_back({-10, -10, -10});
  positions.push_back({90, 90, 90});
  CellFactory(positions);

  DiffusionGrid* dgrid = new StencilGrid(0, "Kalium", 0.4, 1);

  env->Update();
  dgrid->Initialize();
  dgrid->Update();

  auto dims = dgrid->GetDimensions();

  EXPECT_EQ(-40, dims[0]);
  EXPECT_EQ(-40, dims[2]);
  EXPECT_EQ(-40, dims[4]);
  EXPECT_EQ(140, dims[1]);
  EXPECT_EQ(140, dims[3]);
  EXPECT_EQ(140, dims[5]);

  dgrid->Update();

  dims = dgrid->GetDimensions();

  EXPECT_EQ(-40, dims[0]);
  EXPECT_EQ(-40, dims[2]);
  EXPECT_EQ(-40, dims[4]);
  EXPECT_EQ(140, dims[1]);
  EXPECT_EQ(140, dims[3]);
  EXPECT_EQ(140, dims[5]);

  delete dgrid;
}

// Create a 5x5x5 diffusion grid, with a substance being
// added at center box 2,2,2, causing a symmetrical diffusion
TEST(DiffusionTest, LeakingEdge) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();

  DiffusionGrid* dgrid = new StencilGrid(0, "Kalium", 0.4, 0, 5);

  dgrid->Initialize();
  dgrid->SetConcentrationThreshold(1e15);

  for (int i = 0; i < 100; i++) {
    dgrid->ChangeConcentrationBy({{0, 0, 0}}, 4);
    dgrid->DiffuseWithOpenEdge(1.0);
    dgrid->CalculateGradient();
  }

  // Get concentrations and gradients after 100 time steps
  auto conc = dgrid->GetAllConcentrations();
  auto grad = dgrid->GetAllGradients();

  std::array<uint32_t, 3> c = {2, 2, 2};
  std::array<uint32_t, 3> w = {1, 2, 2};
  std::array<uint32_t, 3> e = {3, 2, 2};
  std::array<uint32_t, 3> n = {2, 1, 2};
  std::array<uint32_t, 3> s = {2, 3, 2};
  std::array<uint32_t, 3> t = {2, 2, 1};
  std::array<uint32_t, 3> b = {2, 2, 3};
  std::array<uint32_t, 3> rand1_a = {0, 0, 0};
  std::array<uint32_t, 3> rand1_b = {4, 4, 4};
  std::array<uint32_t, 3> rand2_a = {4, 4, 2};
  std::array<uint32_t, 3> rand2_b = {0, 0, 2};

  auto eps = abs_error<double>::value;

  double v1 = 9.7267657389657938;
  double v2 = 3.7281869469803648;
  double v3 = 0.12493663388071227;
  double v4 = 0.32563083857294983;
  double v5 = 0.08620958617166545;

  EXPECT_NEAR(v1, conc[dgrid->GetBoxIndex(c)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(e)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(w)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(n)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(s)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(t)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(b)], eps);
  EXPECT_NEAR(v3, conc[dgrid->GetBoxIndex(rand1_a)], eps);
  EXPECT_NEAR(v3, conc[dgrid->GetBoxIndex(rand1_b)], eps);
  EXPECT_NEAR(v4, conc[dgrid->GetBoxIndex(rand2_a)], eps);
  EXPECT_NEAR(v4, conc[dgrid->GetBoxIndex(rand2_b)], eps);

  EXPECT_NEAR(0.0, grad[3 * (dgrid->GetBoxIndex(c)) + 1], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(e)) + 0], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(w)) + 0], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(n)) + 1], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(s)) + 1], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(t)) + 2], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(b)) + 2], eps);

  delete dgrid;
}

// Create a 5x5x5 diffusion grid, with a substance being
// added at center box 2,2,2, causing a symmetrical diffusion
TEST(DiffusionTest, ClosedEdge) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  DiffusionGrid* dgrid = new StencilGrid(0, "Kalium", 0.4, 0, 5);

  dgrid->Initialize();
  dgrid->SetConcentrationThreshold(1e15);

  for (int i = 0; i < 100; i++) {
    dgrid->ChangeConcentrationBy({{0, 0, 0}}, 4);
    // Note: the argument in Diffuse Edge is basically ignored because
    // StencilGrid does not allow specific time steps.
    dgrid->DiffuseWithClosedEdge(1.0);
    dgrid->CalculateGradient();
  }

  // Get concentrations and gradients after 100 time steps
  auto conc = dgrid->GetAllConcentrations();
  auto grad = dgrid->GetAllGradients();

  std::array<uint32_t, 3> c = {2, 2, 2};
  std::array<uint32_t, 3> w = {1, 2, 2};
  std::array<uint32_t, 3> e = {3, 2, 2};
  std::array<uint32_t, 3> n = {2, 1, 2};
  std::array<uint32_t, 3> s = {2, 3, 2};
  std::array<uint32_t, 3> t = {2, 2, 1};
  std::array<uint32_t, 3> b = {2, 2, 3};
  std::array<uint32_t, 3> rand1_a = {0, 0, 0};
  std::array<uint32_t, 3> rand1_b = {4, 4, 4};
  std::array<uint32_t, 3> rand2_a = {4, 4, 2};
  std::array<uint32_t, 3> rand2_b = {0, 0, 2};

  auto eps = abs_error<double>::value;

  double v1 = 11.717698164878922;
  double v2 = 5.7977258086605303;
  double v3 = 2.4379152740053867;
  double v4 = 2.7287519978558121;
  double v5 = 0.081744730821864647;

  EXPECT_NEAR(v1, conc[dgrid->GetBoxIndex(c)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(e)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(w)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(n)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(s)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(t)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(b)], eps);
  EXPECT_NEAR(v3, conc[dgrid->GetBoxIndex(rand1_a)], eps);
  EXPECT_NEAR(v3, conc[dgrid->GetBoxIndex(rand1_b)], eps);
  EXPECT_NEAR(v4, conc[dgrid->GetBoxIndex(rand2_a)], eps);
  EXPECT_NEAR(v4, conc[dgrid->GetBoxIndex(rand2_b)], eps);

  EXPECT_NEAR(0.0, grad[3 * (dgrid->GetBoxIndex(c)) + 1], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(e)) + 0], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(w)) + 0], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(n)) + 1], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(s)) + 1], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(t)) + 2], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(b)) + 2], eps);

  delete dgrid;
}

// Tests if the concentration / gradient values are correctly copied
// after the env has grown and DiffusionGrid::CopyOldData is called
TEST(DiffusionTest, CopyOldData) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
  };
  Simulation simulation(TEST_NAME, set_param);
  auto* param = simulation.param_;
  simulation.GetEnvironment()->Update();
  DiffusionGrid* dgrid = new StencilGrid(0, "Kalium", 0.4, 0, 5);

  dgrid->Initialize();
  dgrid->SetConcentrationThreshold(1e15);

  for (int i = 0; i < 100; i++) {
    dgrid->ChangeConcentrationBy({{0, 0, 0}}, 4);
    // Note: the argument in Diffuse Edge is basically ignored because
    // StencilGrid does not allow specific time steps.
    dgrid->DiffuseWithOpenEdge(1.0);
    dgrid->CalculateGradient();
  }

  // Grow grid artificially
  param->min_bound = -140;
  param->max_bound = 140;
  simulation.GetEnvironment()->Update();
  dgrid->Update();

  // Get concentrations and gradients after 100 time steps
  auto conc = dgrid->GetAllConcentrations();
  auto grad = dgrid->GetAllGradients();

  std::array<uint32_t, 3> c = {3, 3, 3};
  std::array<uint32_t, 3> w = {2, 3, 3};
  std::array<uint32_t, 3> e = {4, 3, 3};
  std::array<uint32_t, 3> n = {3, 2, 3};
  std::array<uint32_t, 3> s = {3, 4, 3};
  std::array<uint32_t, 3> t = {3, 3, 2};
  std::array<uint32_t, 3> b = {3, 3, 4};
  std::array<uint32_t, 3> rand1_a = {1, 1, 1};
  std::array<uint32_t, 3> rand1_b = {5, 5, 5};
  std::array<uint32_t, 3> rand2_a = {5, 5, 3};
  std::array<uint32_t, 3> rand2_b = {1, 1, 3};

  auto eps = abs_error<double>::value;

  double v1 = 9.7267657389657938;
  double v2 = 3.7281869469803648;
  double v3 = 0.12493663388071227;
  double v4 = 0.32563083857294983;
  double v5 = 0.08620958617166545;

  EXPECT_NEAR(v1, conc[dgrid->GetBoxIndex(c)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(e)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(w)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(n)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(s)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(t)], eps);
  EXPECT_NEAR(v2, conc[dgrid->GetBoxIndex(b)], eps);
  EXPECT_NEAR(v3, conc[dgrid->GetBoxIndex(rand1_a)], eps);
  EXPECT_NEAR(v3, conc[dgrid->GetBoxIndex(rand1_b)], eps);
  EXPECT_NEAR(v4, conc[dgrid->GetBoxIndex(rand2_a)], eps);
  EXPECT_NEAR(v4, conc[dgrid->GetBoxIndex(rand2_b)], eps);

  EXPECT_NEAR(0.0, grad[3 * (dgrid->GetBoxIndex(c)) + 1], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(e)) + 0], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(w)) + 0], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(n)) + 1], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(s)) + 1], eps);
  EXPECT_NEAR(v5, grad[3 * (dgrid->GetBoxIndex(t)) + 2], eps);
  EXPECT_NEAR(-v5, grad[3 * (dgrid->GetBoxIndex(b)) + 2], eps);

  delete dgrid;
}

#ifdef USE_DICT

// Test if all the data members of the diffusion grid are correctly serialized
// and deserialzed with I/O
TEST(DiffusionTest, IOTest) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -50;
    param->max_bound = 50;
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  remove(ROOTFILE);

  StencilGrid* dgrid = new StencilGrid(0, "Kalium", 0.6, 0);

  // Create a 100x100x100 diffusion grid with 20 boxes per dimension
  dgrid->Initialize();
  dgrid->SetConcentrationThreshold(42);
  dgrid->SetDecayConstant(0.01);

  // write to root file
  WritePersistentObject(ROOTFILE, "dgrid", *dgrid, "new");

  // read back
  StencilGrid* restored_dgrid = nullptr;
  GetPersistentObject(ROOTFILE, "dgrid", restored_dgrid);

  auto eps = abs_error<double>::value;

  EXPECT_EQ("Kalium", restored_dgrid->GetSubstanceName());
  EXPECT_EQ(10, restored_dgrid->GetBoxLength());
  EXPECT_EQ(42, restored_dgrid->GetConcentrationThreshold());
  EXPECT_NEAR(0.4, restored_dgrid->GetDiffusionCoefficients()[0], eps);
  EXPECT_NEAR(0.1, restored_dgrid->GetDiffusionCoefficients()[1], eps);
  EXPECT_NEAR(0.1, restored_dgrid->GetDiffusionCoefficients()[2], eps);
  EXPECT_NEAR(0.1, restored_dgrid->GetDiffusionCoefficients()[3], eps);
  EXPECT_NEAR(0.1, restored_dgrid->GetDiffusionCoefficients()[4], eps);
  EXPECT_NEAR(0.1, restored_dgrid->GetDiffusionCoefficients()[5], eps);
  EXPECT_NEAR(0.1, restored_dgrid->GetDiffusionCoefficients()[6], eps);
  EXPECT_NEAR(0.01, restored_dgrid->GetDecayConstant(), eps);
  EXPECT_EQ(-50, restored_dgrid->GetDimensions()[0]);
  EXPECT_EQ(-50, restored_dgrid->GetDimensions()[2]);
  EXPECT_EQ(-50, restored_dgrid->GetDimensions()[4]);
  EXPECT_EQ(50, restored_dgrid->GetDimensions()[1]);
  EXPECT_EQ(50, restored_dgrid->GetDimensions()[3]);
  EXPECT_EQ(50, restored_dgrid->GetDimensions()[5]);
  EXPECT_EQ(11u, restored_dgrid->GetNumBoxesArray()[0]);
  EXPECT_EQ(11u, restored_dgrid->GetNumBoxesArray()[1]);
  EXPECT_EQ(11u, restored_dgrid->GetNumBoxesArray()[2]);
  EXPECT_EQ(1331u, restored_dgrid->GetNumBoxes());
  EXPECT_EQ(11, restored_dgrid->GetResolution());

  remove(ROOTFILE);
  delete dgrid;
}

#endif  // USE_DICT

Double3 GetRealCoordinates(const std::array<uint32_t, 3>& bc1,
                           const std::array<uint32_t, 3>& bc2, double bl) {
  Double3 ret;
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

TEST(DISABLED_DiffusionTest, WrongParameters) {
  ASSERT_DEATH(
      {
        StencilGrid dgrid(0, "Kalium", 1, 0.5, 51);
        dgrid.Initialize();
      },
      ".*unphysical behavior*");
}  // namespace bdm

TEST(DiffusionTest, CorrectParameters) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 100;
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  StencilGrid dgrid(0, "Kalium", 1, 0.5, 6);
  dgrid.Initialize();
}

TEST(DiffusionTest, EulerConvergence) {
  double simulation_time_step{1.0};
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->diffusion_boundary_condition = "closed";
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();

  double diff_coef = 0.5;
  DiffusionGrid* dgrid2 = new EulerGrid(0, "Kalium1", diff_coef, 0, 21);
  DiffusionGrid* dgrid4 = new EulerGrid(1, "Kalium4", diff_coef, 0, 41);
  DiffusionGrid* dgrid8 = new EulerGrid(2, "Kalium8", diff_coef, 0, 81);

  dgrid2->Initialize();
  dgrid4->Initialize();
  dgrid8->Initialize();

  dgrid2->SetConcentrationThreshold(1e15);
  dgrid4->SetConcentrationThreshold(1e15);
  dgrid8->SetConcentrationThreshold(1e15);

  // instantaneous point source
  int init = 1e5;
  Double3 source = {{0, 0, 0}};
  dgrid2->ChangeConcentrationBy(source, init / pow(dgrid2->GetBoxLength(), 3));
  dgrid4->ChangeConcentrationBy(source, init / pow(dgrid4->GetBoxLength(), 3));
  dgrid8->ChangeConcentrationBy(source, init / pow(dgrid8->GetBoxLength(), 3));

  auto conc2 = dgrid2->GetAllConcentrations();
  auto conc4 = dgrid4->GetAllConcentrations();
  auto conc8 = dgrid8->GetAllConcentrations();

  Double3 marker = {10.0, 10.0, 10.0};

  int tot = 100;
  for (int t = 0; t < tot; t++) {
    dgrid2->Diffuse(simulation_time_step);
    dgrid4->Diffuse(simulation_time_step);
    dgrid8->Diffuse(simulation_time_step);
  }

  auto rc2 = GetRealCoordinates(dgrid2->GetBoxCoordinates(source),
                                dgrid2->GetBoxCoordinates(marker),
                                dgrid2->GetBoxLength());
  auto rc4 = GetRealCoordinates(dgrid4->GetBoxCoordinates(source),
                                dgrid4->GetBoxCoordinates(marker),
                                dgrid4->GetBoxLength());
  auto rc8 = GetRealCoordinates(dgrid8->GetBoxCoordinates(source),
                                dgrid8->GetBoxCoordinates(marker),
                                dgrid8->GetBoxLength());

  auto real_val2 =
      CalculateAnalyticalSolution(init, rc2[0], rc2[1], rc2[2], diff_coef, tot);
  auto real_val4 =
      CalculateAnalyticalSolution(init, rc4[0], rc4[1], rc4[2], diff_coef, tot);
  auto real_val8 =
      CalculateAnalyticalSolution(init, rc8[0], rc8[1], rc8[2], diff_coef, tot);

  auto error2 = std::abs(real_val2 - conc2[dgrid2->GetBoxIndex(marker)]) /
                std::abs(real_val2);
  auto error4 = std::abs(real_val4 - conc4[dgrid4->GetBoxIndex(marker)]) /
                std::abs(real_val4);
  auto error8 = std::abs(real_val8 - conc8[dgrid8->GetBoxIndex(marker)]) /
                std::abs(real_val8);

  EXPECT_TRUE(error4 < error2);
  EXPECT_TRUE(error8 < error4);
  EXPECT_NEAR(error8, 0.01, 0.005);

  delete dgrid2;
  delete dgrid4;
  delete dgrid8;
}

TEST(DiffusionTest, DynamicTimeStepping) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->simulation_time_step = 0.1;
  };
  Simulation simulation(TEST_NAME, set_param);

  simulation.GetEnvironment()->Update();
  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();
  auto* scheduler = simulation.GetScheduler();
  auto diff_op = scheduler->GetOps("diffusion")[0];
  diff_op->frequency_ = 2;

  // Create one cell at a random position
  auto construct = [](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound, 1,
                                       construct);

  // Define the substances in our simulation
  ModelInitializer::DefineSubstance(0, "Substance", 0.5, 0.1, 10);

  // Initialize the substance according to a GaussianBand along the x-axis
  ModelInitializer::InitializeSubstance(0, GaussianBand(125, 50, Axis::kXAxis));

  // Simulate for one timestep
  simulation.GetEnvironment()->Update();
  scheduler->Simulate(3);

  // Test if the timestep is set correctly and if the updates occur at the right
  // Time. Note that the frequency_ is set with GetSimulatedSteps()%frequency_.
  // The following illustrates what happens below.
  // Timestep      |0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|
  // %2            |Y| |Y| |Y| |Y| |Y| | Y|  | Y|  | Y|  | Y|
  // %3            |Y| | |Y| | |Y| | |Y|  |  | Y|  |  | Y|  |
  // %5            |Y| | | | |Y| | | | | Y|  |  |  |  | Y|  |
  // grid updates  |Y| |Y| | |Y| | | | | Y|  | Y|  |  | Y|  |
  // set frequq    |2| | |5| | | | | | |  | 3|  |  |  |  |  |
  // The function get LastTimeStep must return the (#no_spaces+1)*timestep,
  // where #no_spaces counts the empty boxes in between the Y's in grid updates.
  // The colum grid updates always has Y according to the last set frequency.
  // We chose our measurement points such that they occur one timestep after
  // the update.
  auto* dgrid = rm->GetDiffusionGrid(0);
  EXPECT_FLOAT_EQ(0.2, dgrid->GetLastTimestep());
  diff_op->frequency_ = 5;
  scheduler->Simulate(3);
  EXPECT_FLOAT_EQ(0.3, dgrid->GetLastTimestep());
  scheduler->Simulate(5);
  EXPECT_FLOAT_EQ(0.5, dgrid->GetLastTimestep());
  diff_op->frequency_ = 3;
  scheduler->Simulate(2);
  EXPECT_FLOAT_EQ(0.2, dgrid->GetLastTimestep());
  scheduler->Simulate(3);
  EXPECT_FLOAT_EQ(0.3, dgrid->GetLastTimestep());
}

TEST(DISABLED_DiffusionTest, RungeKuttaConvergence) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->diffusion_method = "runga-kutta";
  };
  Simulation simulation(TEST_NAME, set_param);
  double diff_coef = 0.5;
  DiffusionGrid* dgrid2 = new RungaKuttaGrid(0, "Kalium1", diff_coef, 0, 21);
  DiffusionGrid* dgrid4 = new RungaKuttaGrid(1, "Kalium4", diff_coef, 0, 41);
  DiffusionGrid* dgrid8 = new RungaKuttaGrid(2, "Kalium8", diff_coef, 0, 81);

  dgrid2->Initialize();
  dgrid4->Initialize();
  dgrid8->Initialize();

  dgrid2->SetConcentrationThreshold(1e15);
  dgrid4->SetConcentrationThreshold(1e15);
  dgrid8->SetConcentrationThreshold(1e15);

  // instantaneous point source
  int init = 1e5;
  Double3 source = {{0, 0, 0}};
  dgrid2->ChangeConcentrationBy(source, init / pow(dgrid2->GetBoxLength(), 3));
  dgrid4->ChangeConcentrationBy(source, init / pow(dgrid4->GetBoxLength(), 3));
  dgrid8->ChangeConcentrationBy(source, init / pow(dgrid8->GetBoxLength(), 3));

  auto conc2 = dgrid2->GetAllConcentrations();
  auto conc4 = dgrid4->GetAllConcentrations();
  auto conc8 = dgrid8->GetAllConcentrations();

  Double3 marker = {10.0, 10.0, 10.0};

  int tot = 100;
  for (int t = 0; t < tot; t++) {
    dgrid2->DiffuseWithClosedEdge(1.0);
    dgrid4->DiffuseWithClosedEdge(1.0);
    dgrid8->DiffuseWithClosedEdge(1.0);
  }

  auto rc2 = GetRealCoordinates(dgrid2->GetBoxCoordinates(source),
                                dgrid2->GetBoxCoordinates(marker),
                                dgrid2->GetBoxLength());
  auto rc4 = GetRealCoordinates(dgrid4->GetBoxCoordinates(source),
                                dgrid4->GetBoxCoordinates(marker),
                                dgrid4->GetBoxLength());
  auto rc8 = GetRealCoordinates(dgrid8->GetBoxCoordinates(source),
                                dgrid8->GetBoxCoordinates(marker),
                                dgrid8->GetBoxLength());

  auto real_val2 =
      CalculateAnalyticalSolution(init, rc2[0], rc2[1], rc2[2], diff_coef, tot);
  auto real_val4 =
      CalculateAnalyticalSolution(init, rc4[0], rc4[1], rc4[2], diff_coef, tot);
  auto real_val8 =
      CalculateAnalyticalSolution(init, rc8[0], rc8[1], rc8[2], diff_coef, tot);

  auto error2 = std::abs(real_val2 - conc2[dgrid2->GetBoxIndex(marker)]) /
                std::abs(real_val2);
  auto error4 = std::abs(real_val4 - conc4[dgrid4->GetBoxIndex(marker)]) /
                std::abs(real_val4);
  auto error8 = std::abs(real_val8 - conc8[dgrid8->GetBoxIndex(marker)]) /
                std::abs(real_val8);

  EXPECT_TRUE(error4 < error2);
  EXPECT_TRUE(error8 < error4);
  EXPECT_NEAR(error8, 0.01, 0.005);

  delete dgrid2;
  delete dgrid4;
  delete dgrid8;
}

#ifdef USE_PARAVIEW

// Github Actions does not support OpenGL 3.3.
// Therefore, pvpython crashes.
// Renable this test after this issue has been resolved.
TEST(DISABLED_DiffusionTest, ModelInitializer) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    Param::VisualizeDiffusion vd;
    vd.name = "Substance_1";

    param->export_visualization = true;
    param->visualize_diffusion.push_back(vd);
  };
  Simulation sim(TEST_NAME, set_param);
  auto* rm = sim.GetResourceManager();

  enum Substances { kSubstance0, kSubstance1, kSubstance2 };

  // Define the substances in a different order than the enum
  ModelInitializer::DefineSubstance(kSubstance0, "Substance_0", 0.5, 0);
  ModelInitializer::DefineSubstance(kSubstance2, "Substance_2", 0.5, 0);
  ModelInitializer::DefineSubstance(kSubstance1, "Substance_1", 0.5, 0);

  // Initialize one of the substances
  double mean = 0;
  double sigma = 5;
  ModelInitializer::InitializeSubstance(kSubstance1,
                                        GaussianBand(mean, sigma, kXAxis));

  rm->GetDiffusionGrid(kSubstance0)->Initialize();
  rm->GetDiffusionGrid(kSubstance1)->Initialize();
  rm->GetDiffusionGrid(kSubstance2)->Initialize();
  rm->GetDiffusionGrid(kSubstance0)->RunInitializers();
  rm->GetDiffusionGrid(kSubstance1)->RunInitializers();
  rm->GetDiffusionGrid(kSubstance2)->RunInitializers();

  // Write diffusion visualization to file
  sim.Simulate(1);

  // Read back from file
  vtkSmartPointer<vtkXMLImageDataReader> reader =
      vtkSmartPointer<vtkXMLImageDataReader>::New();
  auto filename = Concat(sim.GetOutputDir(), "/Substance_1-0_0.vti");
  if (!FileExists(filename.c_str())) {
    std::cout << filename << " was not generated!" << std::endl;
    FAIL();
  }
  reader->SetFileName(filename.c_str());
  reader->Update();
  vtkImageData* vtk_dgrid = reader->GetOutput();
  vtkAbstractArray* abstract_array =
      vtk_dgrid->GetPointData()->GetArray("Substance Concentration");
  vtkDoubleArray* conc = vtkArrayDownCast<vtkDoubleArray>(abstract_array);

  double expected = ROOT::Math::normal_pdf(0, sigma, mean);
  Double3 marker = {0, 0, 0};
  size_t idx = rm->GetDiffusionGrid(kSubstance1)->GetBoxIndex(marker);
  EXPECT_NEAR(expected, conc->GetTuple(idx)[0], 1e-9);
  remove(filename.c_str());
}

#endif  // USE_PARAVIEW

}  // namespace bdm
