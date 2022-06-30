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

#include <fstream>

#include "core/agent/cell.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/diffusion/euler_depletion_grid.h"
#include "core/diffusion/euler_grid.h"
#include "core/diffusion/runge_kutta_grid.h"
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

// Adds a displacement to a (grid) position.
void AddDisplacement(std::array<uint32_t, 3>& position,
                     const std::array<uint32_t, 3>& displacement) {
  position[0] += displacement[0];
  position[1] += displacement[1];
  position[2] += displacement[2];
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

  DiffusionGrid* dgrid = new EulerGrid(0, "Kalium", 0.4, 0, 2);

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

  DiffusionGrid* dgrid = new EulerGrid(0, "Kalium", 0.4, 0, 6);

  env->ForcedUpdate();
  dgrid->Initialize();

  std::vector<Double3> positions_2;
  positions_2.push_back({-30, -10, -10});
  positions_2.push_back({90, 150, 90});
  CellFactory(positions_2);

  env->ForcedUpdate();

  dgrid->Update();

  auto d_dims = dgrid->GetDimensions();

  EXPECT_EQ(-90, d_dims[0]);
  EXPECT_EQ(-90, d_dims[2]);
  EXPECT_EQ(-90, d_dims[4]);
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

  DiffusionGrid* dgrid = new EulerGrid(0, "Kalium", 0.4, 1);

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
  DiffusionGrid* dgrid = new EulerGrid(0, "Kalium", 0.4, 0, 5);

  dgrid->Initialize();
  dgrid->SetUpperThreshold(1e15);
  dgrid->SetLowerThreshold(-1e15);

  for (int i = 0; i < 100; i++) {
    dgrid->ChangeConcentrationBy({{0, 0, 0}}, 4);
    dgrid->DiffuseWithOpenEdge(1.0);
    dgrid->CalculateGradient();
  }

  // Get number of boxes before rescaling
  size_t grid_size_1 = dgrid->GetNumBoxes();

  // Define grid positions that we want to analyze
  std::array<uint32_t, 3> center = {2, 2, 2};
  std::array<uint32_t, 3> west = {1, 2, 3};
  std::array<uint32_t, 3> east = {3, 2, 2};
  std::array<uint32_t, 3> north = {2, 1, 2};
  std::array<uint32_t, 3> south = {2, 3, 2};
  std::array<uint32_t, 3> bottom = {2, 2, 1};
  std::array<uint32_t, 3> top = {2, 2, 3};
  std::array<uint32_t, 3> rand1_a = {0, 0, 0};
  std::array<uint32_t, 3> rand1_b = {4, 4, 4};
  std::array<uint32_t, 3> rand2_a = {4, 4, 2};
  std::array<uint32_t, 3> rand2_b = {0, 0, 2};

  // Get concentrations and gradients after 100 time steps
  auto conc_1 = dgrid->GetAllConcentrations();
  auto grad_1 = dgrid->GetAllGradients();

  // Readout concentration values
  double concentration_center = conc_1[dgrid->GetBoxIndex(center)];
  double concentration_west = conc_1[dgrid->GetBoxIndex(west)];
  double concentration_east = conc_1[dgrid->GetBoxIndex(east)];
  double concentration_north = conc_1[dgrid->GetBoxIndex(north)];
  double concentration_south = conc_1[dgrid->GetBoxIndex(south)];
  double concentration_bottom = conc_1[dgrid->GetBoxIndex(bottom)];
  double concentration_top = conc_1[dgrid->GetBoxIndex(top)];
  double concentration_rand1_a = conc_1[dgrid->GetBoxIndex(rand1_a)];
  double concentration_rand1_b = conc_1[dgrid->GetBoxIndex(rand1_b)];
  double concentration_rand2_a = conc_1[dgrid->GetBoxIndex(rand2_a)];
  double concentration_rand2_b = conc_1[dgrid->GetBoxIndex(rand2_b)];

  // Readout gradient values
  double gradient_x_center = grad_1[3 * dgrid->GetBoxIndex(center) + 0];
  double gradient_y_center = grad_1[3 * dgrid->GetBoxIndex(center) + 1];
  double gradient_z_center = grad_1[3 * dgrid->GetBoxIndex(center) + 2];
  double gradient_x_west = grad_1[3 * dgrid->GetBoxIndex(west) + 0];
  double gradient_y_west = grad_1[3 * dgrid->GetBoxIndex(west) + 1];
  double gradient_z_west = grad_1[3 * dgrid->GetBoxIndex(west) + 2];
  double gradient_x_east = grad_1[3 * dgrid->GetBoxIndex(east) + 0];
  double gradient_y_east = grad_1[3 * dgrid->GetBoxIndex(east) + 1];
  double gradient_z_east = grad_1[3 * dgrid->GetBoxIndex(east) + 2];
  double gradient_x_north = grad_1[3 * dgrid->GetBoxIndex(north) + 0];
  double gradient_y_north = grad_1[3 * dgrid->GetBoxIndex(north) + 1];
  double gradient_z_north = grad_1[3 * dgrid->GetBoxIndex(north) + 2];
  double gradient_x_south = grad_1[3 * dgrid->GetBoxIndex(south) + 0];
  double gradient_y_south = grad_1[3 * dgrid->GetBoxIndex(south) + 1];
  double gradient_z_south = grad_1[3 * dgrid->GetBoxIndex(south) + 2];
  double gradient_x_bottom = grad_1[3 * dgrid->GetBoxIndex(bottom) + 0];
  double gradient_y_bottom = grad_1[3 * dgrid->GetBoxIndex(bottom) + 1];
  double gradient_z_bottom = grad_1[3 * dgrid->GetBoxIndex(bottom) + 2];
  double gradient_x_top = grad_1[3 * dgrid->GetBoxIndex(top) + 0];
  double gradient_y_top = grad_1[3 * dgrid->GetBoxIndex(top) + 1];
  double gradient_z_top = grad_1[3 * dgrid->GetBoxIndex(top) + 2];

  // Grow grid artificially
  param->min_bound = -140;
  param->max_bound = 140;
  simulation.GetEnvironment()->ForcedUpdate();
  dgrid->Update();

  // Get number of boxes after rescaling
  size_t grid_size_2 = dgrid->GetNumBoxes();

  // Get concentrations and gradients after 100 time steps
  auto conc_2 = dgrid->GetAllConcentrations();
  auto grad_2 = dgrid->GetAllGradients();

  // Compute new grid position resulting from the halo introduced by CopyOldData
  std::array<uint32_t, 3> displacement = {1, 1, 1};
  AddDisplacement(center, displacement);
  AddDisplacement(west, displacement);
  AddDisplacement(east, displacement);
  AddDisplacement(north, displacement);
  AddDisplacement(south, displacement);
  AddDisplacement(bottom, displacement);
  AddDisplacement(top, displacement);
  AddDisplacement(rand1_a, displacement);
  AddDisplacement(rand1_b, displacement);
  AddDisplacement(rand2_a, displacement);
  AddDisplacement(rand2_b, displacement);

  // Test if diffusion grid was rescaled
  EXPECT_NE(grid_size_1, grid_size_2);

  // Test if values are copied correctly
  EXPECT_DOUBLE_EQ(concentration_center, conc_2[dgrid->GetBoxIndex(center)]);
  EXPECT_DOUBLE_EQ(concentration_west, conc_2[dgrid->GetBoxIndex(west)]);
  EXPECT_DOUBLE_EQ(concentration_east, conc_2[dgrid->GetBoxIndex(east)]);
  EXPECT_DOUBLE_EQ(concentration_north, conc_2[dgrid->GetBoxIndex(north)]);
  EXPECT_DOUBLE_EQ(concentration_south, conc_2[dgrid->GetBoxIndex(south)]);
  EXPECT_DOUBLE_EQ(concentration_bottom, conc_2[dgrid->GetBoxIndex(bottom)]);
  EXPECT_DOUBLE_EQ(concentration_top, conc_2[dgrid->GetBoxIndex(top)]);
  EXPECT_DOUBLE_EQ(concentration_rand1_a, conc_2[dgrid->GetBoxIndex(rand1_a)]);
  EXPECT_DOUBLE_EQ(concentration_rand1_b, conc_2[dgrid->GetBoxIndex(rand1_b)]);
  EXPECT_DOUBLE_EQ(concentration_rand2_a, conc_2[dgrid->GetBoxIndex(rand2_a)]);
  EXPECT_DOUBLE_EQ(concentration_rand2_b, conc_2[dgrid->GetBoxIndex(rand2_b)]);

  // Test if gradients are copied correctly
  EXPECT_DOUBLE_EQ(gradient_x_center,
                   grad_2[3 * dgrid->GetBoxIndex(center) + 0]);
  EXPECT_DOUBLE_EQ(gradient_y_center,
                   grad_2[3 * dgrid->GetBoxIndex(center) + 1]);
  EXPECT_DOUBLE_EQ(gradient_z_center,
                   grad_2[3 * dgrid->GetBoxIndex(center) + 2]);
  EXPECT_DOUBLE_EQ(gradient_x_west, grad_2[3 * dgrid->GetBoxIndex(west) + 0]);
  EXPECT_DOUBLE_EQ(gradient_y_west, grad_2[3 * dgrid->GetBoxIndex(west) + 1]);
  EXPECT_DOUBLE_EQ(gradient_z_west, grad_2[3 * dgrid->GetBoxIndex(west) + 2]);
  EXPECT_DOUBLE_EQ(gradient_x_east, grad_2[3 * dgrid->GetBoxIndex(east) + 0]);
  EXPECT_DOUBLE_EQ(gradient_y_east, grad_2[3 * dgrid->GetBoxIndex(east) + 1]);
  EXPECT_DOUBLE_EQ(gradient_z_east, grad_2[3 * dgrid->GetBoxIndex(east) + 2]);
  EXPECT_DOUBLE_EQ(gradient_x_north, grad_2[3 * dgrid->GetBoxIndex(north) + 0]);
  EXPECT_DOUBLE_EQ(gradient_y_north, grad_2[3 * dgrid->GetBoxIndex(north) + 1]);
  EXPECT_DOUBLE_EQ(gradient_z_north, grad_2[3 * dgrid->GetBoxIndex(north) + 2]);
  EXPECT_DOUBLE_EQ(gradient_x_south, grad_2[3 * dgrid->GetBoxIndex(south) + 0]);
  EXPECT_DOUBLE_EQ(gradient_y_south, grad_2[3 * dgrid->GetBoxIndex(south) + 1]);
  EXPECT_DOUBLE_EQ(gradient_z_south, grad_2[3 * dgrid->GetBoxIndex(south) + 2]);
  EXPECT_DOUBLE_EQ(gradient_x_bottom,
                   grad_2[3 * dgrid->GetBoxIndex(bottom) + 0]);
  EXPECT_DOUBLE_EQ(gradient_y_bottom,
                   grad_2[3 * dgrid->GetBoxIndex(bottom) + 1]);
  EXPECT_DOUBLE_EQ(gradient_z_bottom,
                   grad_2[3 * dgrid->GetBoxIndex(bottom) + 2]);
  EXPECT_DOUBLE_EQ(gradient_x_top, grad_2[3 * dgrid->GetBoxIndex(top) + 0]);
  EXPECT_DOUBLE_EQ(gradient_y_top, grad_2[3 * dgrid->GetBoxIndex(top) + 1]);
  EXPECT_DOUBLE_EQ(gradient_z_top, grad_2[3 * dgrid->GetBoxIndex(top) + 2]);

  delete dgrid;
}

// Create a 5x5x5 diffusion grid, with a substance being
// added at center box 2,2,2, causing a symmetrical diffusion
TEST(DiffusionTest, Thresholds) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  DiffusionGrid* dgrid = new EulerGrid(0, "Kalium", 0.4, 0, 50);

  Double3 pos_upper({{0, 0, 0}});
  Double3 pos_lower({{10, 10, 10}});
  double upper_threshold = 3;
  double lower_threshold = -2;
  dgrid->Initialize();
  dgrid->SetUpperThreshold(upper_threshold);
  dgrid->SetLowerThreshold(lower_threshold);

  EXPECT_EQ(upper_threshold, dgrid->GetUpperThreshold());
  EXPECT_EQ(lower_threshold, dgrid->GetLowerThreshold());

  for (int i = 0; i < 10; i++) {
    dgrid->ChangeConcentrationBy(pos_upper, 1.0);
    dgrid->ChangeConcentrationBy(pos_lower, -1.0);
  }

  EXPECT_DOUBLE_EQ(upper_threshold, dgrid->GetConcentration(pos_upper));
  EXPECT_DOUBLE_EQ(lower_threshold, dgrid->GetConcentration(pos_lower));

  delete dgrid;
}

#ifdef USE_DICT

// Test if all the data members of the diffusion grid are correctly serialized
// and deserialized with I/O
TEST(DiffusionTest, IOTest) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -50;
    param->max_bound = 50;
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  remove(ROOTFILE);

  EulerGrid* dgrid = new EulerGrid(0, "Kalium", 0.6, 0);

  // Create a 100x100x100 diffusion grid with 20 boxes per dimension
  dgrid->Initialize();
  dgrid->SetUpperThreshold(42);
  dgrid->SetLowerThreshold(-42);
  dgrid->SetDecayConstant(0.01);

  // write to root file
  WritePersistentObject(ROOTFILE, "dgrid", *dgrid, "new");

  // read back
  EulerGrid* restored_dgrid = nullptr;
  GetPersistentObject(ROOTFILE, "dgrid", restored_dgrid);

  auto eps = abs_error<double>::value;

  EXPECT_EQ("Kalium", restored_dgrid->GetSubstanceName());
  EXPECT_EQ(10, restored_dgrid->GetBoxLength());
  EXPECT_EQ(42, restored_dgrid->GetUpperThreshold());
  EXPECT_EQ(-42, restored_dgrid->GetLowerThreshold());
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
  EXPECT_EQ(10u, restored_dgrid->GetNumBoxesArray()[0]);
  EXPECT_EQ(10u, restored_dgrid->GetNumBoxesArray()[1]);
  EXPECT_EQ(10u, restored_dgrid->GetNumBoxesArray()[2]);
  EXPECT_EQ(1000u, restored_dgrid->GetNumBoxes());
  EXPECT_EQ(10u, restored_dgrid->GetResolution());

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
        EulerGrid dgrid(0, "Kalium", 1, 0.5, 51);
        dgrid.Initialize();
        dgrid.Diffuse(1.0);
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
  EulerGrid dgrid(0, "Kalium", 1, 0.5, 6);
  dgrid.Initialize();
  dgrid.Diffuse(1.0);
}

TEST(DiffusionTest, EulerConvergenceExponentialDecay) {
  double simulation_time_step{0.1};
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->diffusion_boundary_condition = "closed";
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();

  double diff_coef = 0.0;
  double decay = 0.01;
  DiffusionGrid* dgrid = new EulerGrid(0, "Kalium1", diff_coef, decay, 81);
  dgrid->Initialize();
  dgrid->SetUpperThreshold(1e15);

  // instantaneous point source
  int init = 1e5;
  Double3 source = {{0, 0, 0}};
  dgrid->ChangeConcentrationBy(source, init);
  auto conc2 = dgrid->GetAllConcentrations();
  Double3 marker = {10.0, 10.0, 10.0};

  // Simulate diffusion / exponential decay for `tot` timesteps
  int tot = 100;
  for (int t = 0; t < tot; t++) {
    dgrid->Diffuse(simulation_time_step);
  }

  // If there is no diffusion, each grid point simply executes an independet
  // exponential decay.
  double expected_solution =
      init * std::exp(-decay * tot * simulation_time_step);

  // No diffusing substance -> Solution is 0 if not at source.
  EXPECT_FLOAT_EQ(0.0, conc2[dgrid->GetBoxIndex(marker)]);
  // Expect numeric value of exponential decay to coincide with +/- 0.01% of
  // analytic solution.
  EXPECT_LT(std::abs(expected_solution - conc2[dgrid->GetBoxIndex(source)]) /
                expected_solution,
            0.0001);

  delete dgrid;
}

TEST(DiffusionTest, EulerConvergenceDiffusion) {
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
  DiffusionGrid* dgrid2 = new EulerGrid(0, "Kalium1", diff_coef, 0, 20);
  DiffusionGrid* dgrid4 = new EulerGrid(1, "Kalium4", diff_coef, 0, 40);
  DiffusionGrid* dgrid8 = new EulerGrid(2, "Kalium8", diff_coef, 0, 80);

  dgrid2->Initialize();
  dgrid4->Initialize();
  dgrid8->Initialize();

  dgrid2->SetUpperThreshold(1e15);
  dgrid4->SetUpperThreshold(1e15);
  dgrid8->SetUpperThreshold(1e15);

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

TEST(DiffusionTest, EulerDepletionConvergenceExponentialDecay) {
  double simulation_time_step{0.1};
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->diffusion_boundary_condition = "closed";
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  auto* rm = simulation.GetResourceManager();

  double diff_coef = 0.0;
  double decay_depletes = 0.0;
  double decay_depleted = 0.01;
  int res = 20;
  // Depleting substance is fixed, i.e. no diff and no decay
  auto* dgrid_depletes1 =
      new EulerGrid(0, "MMP", diff_coef, decay_depletes, res);
  auto* dgrid_depletes2 =
      new EulerGrid(1, "TIMP", diff_coef, decay_depletes, res);
  auto* dgrid_depleted =
      new EulerDepletionGrid(2, "ECM", diff_coef, decay_depleted, res);
  dgrid_depletes1->Initialize();
  dgrid_depletes1->SetUpperThreshold(1e15);
  rm->AddDiffusionGrid(dgrid_depletes1);
  dgrid_depletes2->Initialize();
  dgrid_depletes2->SetUpperThreshold(1e15);
  rm->AddDiffusionGrid(dgrid_depletes2);
  dgrid_depleted->Initialize();
  dgrid_depleted->SetUpperThreshold(1e15);
  rm->AddDiffusionGrid(dgrid_depleted);

  // Add depleting substance with its binding coefficient
  std::vector<double> bnd_coeff{0.01, 0.01};
  std::vector<size_t> bnd_subs{dgrid_depletes1->GetSubstanceId(),
                               dgrid_depletes2->GetSubstanceId()};
  dgrid_depleted->SetBindingSubstance(bnd_subs[0], bnd_coeff[0]);
  dgrid_depleted->SetBindingSubstance(bnd_subs[1], bnd_coeff[1]);

  // instantaneous point source
  double init_depleted = 1e2;
  double init_depletes = 1;
  Double3 source = {{0, 0, 0}};
  dgrid_depletes1->ChangeConcentrationBy(source, init_depletes);
  dgrid_depletes2->ChangeConcentrationBy(source, init_depletes);
  dgrid_depleted->ChangeConcentrationBy(source, init_depleted);
  Double3 marker = {50.0, 50.0, 50.0};

  // Simulate diffusion / exponential decay for `tot` timesteps
  int tot = 100;
  for (int t = 0; t < tot; t++) {
    dgrid_depletes1->Diffuse(simulation_time_step);
    dgrid_depletes2->Diffuse(simulation_time_step);
    dgrid_depleted->Diffuse(simulation_time_step);
  }

  auto conc_depletes1 = dgrid_depletes1->GetAllConcentrations();
  auto conc_depletes2 = dgrid_depletes2->GetAllConcentrations();
  auto conc_depleted = dgrid_depleted->GetAllConcentrations();

  // If there is no diffusion, each grid point of the depleted substance simply
  // executes an independet exponential decay due to self and induced depletion
  double total_mu = decay_depleted + init_depletes * bnd_coeff[0] +
                    init_depletes * bnd_coeff[1];
  double expected_solution =
      init_depleted * std::exp(-total_mu * tot * simulation_time_step);

  // No diffusing substance -> Solution is 0 if not at source.
  EXPECT_FLOAT_EQ(0.0, conc_depletes1[dgrid_depletes1->GetBoxIndex(marker)]);
  EXPECT_FLOAT_EQ(0.0, conc_depletes2[dgrid_depletes2->GetBoxIndex(marker)]);
  EXPECT_FLOAT_EQ(0.0, conc_depleted[dgrid_depleted->GetBoxIndex(marker)]);
  // Depleting substances don't decay -> Conc shouldn't change at source
  EXPECT_EQ(init_depletes,
            conc_depletes1[dgrid_depletes1->GetBoxIndex(source)]);
  EXPECT_EQ(init_depletes,
            conc_depletes2[dgrid_depletes2->GetBoxIndex(source)]);
  // Expect numeric value of exponential decay to coincide with +/- 1% of
  // analytic solution.
  EXPECT_LT(std::abs(expected_solution -
                     conc_depleted[dgrid_depleted->GetBoxIndex(source)]) /
                expected_solution,
            0.01);

  // dgrids are deleted by rm's destructor
}

TEST(DiffusionTest, EulerDirichletBoundaries) {
  double simulation_time_step{0.1};
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->diffusion_boundary_condition = "Dirichlet";
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  auto* rm = simulation.GetResourceManager();

  double decay_coef = 0.0;
  double diff_coef = 100.0;
  int res = 20;
  // Depleting substance is fixed, i.e. no diff and no decay
  auto* dgrid = new EulerGrid(0, "Kalium", diff_coef, decay_coef, res);

  dgrid->Initialize();
  dgrid->SetBoundaryConditionType(kDirichlet);

  struct SetMyBoundaries {
    SetMyBoundaries() {}
    double operator()(size_t x, size_t y, size_t z, size_t n) { return 1.0; }
  };

  dgrid->SetBoundaryCondition(SetMyBoundaries());
  dgrid->SetUpperThreshold(1e15);
  rm->AddDiffusionGrid(dgrid);

  // Simulate diffusion / exponential decay for `tot` timesteps
  int tot = 100000;  // ToDo This should probably be lower for final version
  for (int t = 0; t < tot; t++) {
    dgrid->Diffuse(simulation_time_step);
  }

  // After a sufficient amount of iterations with dirichlet boundary conditions
  // equal to 1.0, the concentration should be 1.0 everywhere.
  auto conc = dgrid->GetAllConcentrations();
  double average_concentration = 0.0;
  for (size_t i = 0; i < dgrid->GetNumBoxes(); i++) {
    average_concentration += conc[i];
  }
  average_concentration /= dgrid->GetNumBoxes();
  EXPECT_FLOAT_EQ(average_concentration, 1.0);
}

TEST(DiffusionTest, EulerNeumannZeroBoundaries) {
  double simulation_time_step{0.1};
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->diffusion_boundary_condition = "Neumann";
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  auto* rm = simulation.GetResourceManager();

  double decay_coef = 0.0;
  double diff_coef = 10.0;
  int res = 20;
  double init = 1e5;
  std::vector<Double3> sources;
  sources.push_back({0, 0, 0});
  sources.push_back({50, 50, 50});
  sources.push_back({-50, -50, -50});

  struct SetMyBoundaries {
    SetMyBoundaries() {}
    double operator()(size_t x, size_t y, size_t z, size_t n) { return 0.0; }
  };

  // Test multiple positions for the source
  for (size_t s = 0; s < sources.size(); s++) {
    auto* dgrid = new EulerGrid(0, "Kalium", diff_coef, decay_coef, res);
    dgrid->Initialize();
    dgrid->ChangeConcentrationBy(sources[s], init);
    dgrid->SetBoundaryConditionType(kNeumann);
    dgrid->SetBoundaryCondition(SetMyBoundaries());
    dgrid->SetUpperThreshold(1e15);
    rm->AddDiffusionGrid(dgrid);

    // Simulate diffusion / exponential decay for `tot` timesteps
    int tot = 10000;
    for (int t = 0; t < tot; t++) {
      dgrid->Diffuse(simulation_time_step);
    }
    double expected_solution = 0.0;
    auto conc = dgrid->GetAllConcentrations();
    for (size_t i = 0; i < dgrid->GetNumBoxes(); i++) {
      expected_solution += conc[i];
    }
    EXPECT_LT(std::abs(init - expected_solution) / init, 0.0001);
    rm->RemoveDiffusionGrid(0);
  }
}

/// Test verifies if the diffusion grid is able to handle a Neumann boundary
/// with a non-zero value. We test for -1.0 which should add concentration
/// to the grid.
TEST(DiffusionTest, EulerNeumannNonZeroBoundaries) {
  // Define some parameters & simulation
  double simulation_time_step{0.1};
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = -100;
    param->max_bound = 100;
    param->diffusion_boundary_condition = "Neumann";
  };
  Simulation simulation(TEST_NAME, set_param);
  simulation.GetEnvironment()->Update();
  auto* rm = simulation.GetResourceManager();

  double decay_coef = 0.0;  // no decay
  double diff_coef = 1.0;   // diffusion
  int res = 20;             // some resolution

  /// Normal vector typically points outwards, hence -1.0 adds concentration to
  /// the volume. This test tests if we add concentration to the volume.
  struct SetMyBoundaries {
    SetMyBoundaries() {}
    double operator()(size_t x, size_t y, size_t z, size_t n) { return -1.0; }
  };

  auto* dgrid = new EulerGrid(0, "Kalium", diff_coef, decay_coef, res);
  dgrid->Initialize();
  dgrid->SetBoundaryConditionType(kNeumann);
  dgrid->SetBoundaryCondition(SetMyBoundaries());
  dgrid->SetUpperThreshold(1e15);
  rm->AddDiffusionGrid(dgrid);

  // ----------------------------------------------------------
  // 1. Test if DG was initialized correctly (zero initialized)
  // ----------------------------------------------------------
  auto conc = dgrid->GetAllConcentrations();
  double init = 0.0;
  for (size_t i = 0; i < dgrid->GetNumBoxes(); i++) {
    init += conc[i];
  }
  ASSERT_DOUBLE_EQ(init, 0.0);

  // ----------------------------------------------------------
  // 2. Simulate diffusion and check if concentration is added
  // ----------------------------------------------------------

  double intermediate_concentration_backup = 0.0;
  int tot = 10000;
  for (int t = 0; t < tot; t++) {
    dgrid->Diffuse(simulation_time_step);
    conc = dgrid->GetAllConcentrations();  // get current concentrations.
    double intermediate_concentration_1 = 0.0;
    for (size_t i = 0; i < dgrid->GetNumBoxes(); i++) {
      intermediate_concentration_1 += conc[i];
    }
    if (t == 0) {
      /// Note: We have 20x20x20 boxes, so we have (20x20)x2+(19*18)x4=2168
      /// boxes on the boundary. In the first step, each boundary box should
      /// receive a concentration of
      /// D * (dt / dx^2) * - BC * dx = 1.0 * (0.1 / (10*10)) * -(-1) * 10 = 10^-2.
      /// Thus, the total concentration should be roughly 21.68 (the edges get additional influx), and definitely
      /// bigger than 1.0 .
      EXPECT_GT(intermediate_concentration_1, 1.0);
    } else {
      /// For all further iterations, we simply check if the concentration
      /// increases. Which should be the case for the given BC.
      EXPECT_GT(intermediate_concentration_1,
                intermediate_concentration_backup);
    }
    intermediate_concentration_backup = intermediate_concentration_1;
  }

  rm->RemoveDiffusionGrid(0);
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
  scheduler->Simulate(3);

  // Test if the timestep is set correctly and if the updates occur at the right
  // Time. Note that the frequency_ is set with GetSimulatedSteps()%frequency_.
  // The following illustrates what happens below.
  // Timestep      |0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|
  // %2            |Y| |Y| |Y| |Y| |Y| | Y|  | Y|  | Y|  | Y|
  // %3            |Y| | |Y| | |Y| | |Y|  |  | Y|  |  | Y|  |
  // %5            |Y| | | | |Y| | | | | Y|  |  |  |  | Y|  |
  // grid updates  |Y| |Y| | |Y| | | | | Y|  | Y|  |  | Y|  |
  // set freq      |2| | |5| | | | | | |  | 3|  |  |  |  |  |
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
    param->diffusion_method = "runge-kutta";
  };
  Simulation simulation(TEST_NAME, set_param);
  double diff_coef = 0.5;
  DiffusionGrid* dgrid2 = new RungeKuttaGrid(0, "Kalium1", diff_coef, 20);
  DiffusionGrid* dgrid4 = new RungeKuttaGrid(1, "Kalium4", diff_coef, 40);
  DiffusionGrid* dgrid8 = new RungeKuttaGrid(2, "Kalium8", diff_coef, 80);

  dgrid2->Initialize();
  dgrid4->Initialize();
  dgrid8->Initialize();

  dgrid2->SetUpperThreshold(1e15);
  dgrid4->SetUpperThreshold(1e15);
  dgrid8->SetUpperThreshold(1e15);

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

TEST(DiffusionTest, GradientComputation) {
  // Define closed space with bounds
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 250;
  };

  // Standard simulation objects
  Simulation simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();

  // Create a number of cells at a random position
  uint64_t num_cells = 30;
  auto construct = [](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound,
                                       num_cells, construct);

  // Define the substance for our simulation
  DiffusionGrid* d_grid = nullptr;
  d_grid = new EulerGrid(0, "Substance", 0.0, 0.0, 100);
  rm->AddDiffusionGrid(d_grid);

  // Define scalar field for initialization
  auto scalar_field = [&](double x, double y, double z) {
    return 2 * x / param->max_bound + 4 * std::pow((y / param->max_bound), 2) +
           std::sin(2 * Math::kPi * z / param->max_bound);
  };

  // Define the analytic gradient as a reference solution
  auto gradient_field = [&](const Double3& pos) {
    double y = pos[1];
    double z = pos[2];
    Double3 gradient;
    gradient[0] = 2 / param->max_bound;
    gradient[1] = 8 * y / std::pow(param->max_bound, 2);
    gradient[2] = (2 * Math::kPi / param->max_bound) *
                  std::cos(2 * Math::kPi * z / param->max_bound);
    return gradient;
  };

  // Compute the gradient the scalar field.
  ModelInitializer::InitializeSubstance(0, scalar_field);
  simulation.GetScheduler()->Simulate(1);
  d_grid->CalculateGradient();

  // Define a few dummy positions where to evaluate the gradient
  Double3 pos1{20, 50, 230};
  Double3 pos2{100, 200, 150};
  Double3 pos3{200, 100, 20};

  // Get analytic and numerical gradients
  Double3 grad1, grad2, grad3;
  Double3 expected_grad1 = gradient_field(pos1);
  Double3 expected_grad2 = gradient_field(pos2);
  Double3 expected_grad3 = gradient_field(pos3);
  d_grid->GetGradient(pos1, &grad1, false);
  d_grid->GetGradient(pos2, &grad2, false);
  d_grid->GetGradient(pos3, &grad3, false);
  Double3 err1 = grad1 - expected_grad1;
  Double3 err2 = grad2 - expected_grad2;
  Double3 err3 = grad3 - expected_grad3;
  std::vector<Double3> results{err1, err2, err3};
  std::vector<Double3> expected_gradients{expected_grad1, expected_grad2,
                                          expected_grad3};

  // Compare analytic and numeric gradient
  for (size_t i = 0; i < results.size(); i++) {
    auto res = results[i];
    auto grad = expected_gradients[i];
    EXPECT_LT(abs(res[0] / grad[0]), 1e-5);  // This gradient is very easy
    EXPECT_LT(abs(res[1] / grad[1]), 0.05);  // Require 5 precent accuracy
    EXPECT_LT(abs(res[2] / grad[2]), 0.03);  // Require 3 precent accuracy
  }
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
