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

#include "diffusion_init_test.h"
#include "core/agent/cell.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/environment/environment.h"
#include "core/model_initializer.h"
#include "core/simulation.h"
#include "core/substance_initializers.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

#include "Math/DistFunc.h"

namespace bdm {

enum Substances { kSubstance };

TEST(DiffusionInitTest, GaussianBand) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 250;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();

  // Create one cell at a random position
  auto construct = [](const Real3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound, 1,
                                       construct);

  // Define the substances in our simulation
  ModelInitializer::DefineSubstance(kSubstance, "Substance", 0.5, 0.1, 25);

  // Initialize the substance according to a GaussianBand along the x-axis
  ModelInitializer::InitializeSubstance(kSubstance,
                                        GaussianBand(125, 50, Axis::kXAxis));

  simulation.GetEnvironment()->Update();

  auto* dgrid = rm->GetDiffusionGrid(0);

  // Create data structures, whose size depend on the grid dimensions
  dgrid->Initialize();
  // Initialize data structures with user-defined values
  dgrid->RunInitializers();

  // Define points to evaluate the substance (coincides with the grid points)
  Real3 a = {5, 0, 0};
  Real3 b = {245, 0, 0};
  Real3 c = {135, 0, 0};
  Real3 d = {5, 135, 0};
  Real3 e = {245, 0, 135};
  Real3 f = {135, 135, 135};

  auto eps = abs_error<real_t>::value;

  EXPECT_NEAR(ROOT::Math::normal_pdf(a[0], 50, 125), dgrid->GetValue(a), eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(b[0], 50, 125), dgrid->GetValue(b), eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(c[0], 50, 125), dgrid->GetValue(c), eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(d[0], 50, 125), dgrid->GetValue(d), eps);
  // Should be symmetric, so the two ends should have the same value
  EXPECT_NEAR(ROOT::Math::normal_pdf(e[0], 50, 125), dgrid->GetValue(e), eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(f[0], 50, 125), dgrid->GetValue(f), eps);
}

// Both internal arrays (c1_ and c2_) need to be initialized to avoid unphysical
// effects at the boundary after the first internal swap. See PR #199.
TEST(DiffusionInitTest, InitBothArrays) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 250;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();

  // Create one cell at a random position
  auto construct = [](const Real3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound, 1,
                                       construct);

  // Define the substances in our simulation
  DiffusionGrid* d_grid = nullptr;
  d_grid = new TestGrid(kSubstance, "Substance", 0.0, 0.0, 26);
  rm->AddContinuum(d_grid);

  // Initialize the substance according to a GaussianBand along the x-axis
  auto SetValues = [&](real_t x, real_t y, real_t z) { return 0.5; };
  ModelInitializer::InitializeSubstance(kSubstance, SetValues);

  simulation.GetScheduler()->Simulate(1);

  // Test if all values in c1_ are 0.5 and if all values in c2_ are the same as
  // in c1_.
  auto* test_grid = bdm_static_cast<TestGrid*>(d_grid);
  EXPECT_TRUE(test_grid->ComapareArrayWithValue(0.5));
  EXPECT_TRUE(test_grid->CompareArrays());
}

TEST(DiffusionInitTest, InitDirichlet) {
  auto set_param = [](auto* param) {
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 250;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();

  // Create one cell at a random position
  auto construct = [](const Real3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound, 1,
                                       construct);

  // Define the substances in our simulation
  DiffusionGrid* d_grid = nullptr;
  d_grid = new TestGrid(kSubstance, "Substance", 0.0, 0.0, 26);
  rm->AddContinuum(d_grid);

  // Set Dirichlet boundary conditions with value 0.25
  ModelInitializer::AddBoundaryConditions(
      kSubstance, BoundaryConditionType::kDirichlet,
      std::make_unique<ConstantBoundaryCondition>(0.25));

  // Initialize the substance to 0.5
  auto SetValues = [&](real_t x, real_t y, real_t z) { return 0.5; };
  ModelInitializer::InitializeSubstance(kSubstance, SetValues);

  simulation.GetScheduler()->Simulate(1);

  // Test if all values in c1_ are 0.5 (inner points) and 0 on the boundary.
  // Test if all values in c2_ are the same as in c1_.
  auto* test_grid = bdm_static_cast<TestGrid*>(d_grid);
  EXPECT_FALSE(test_grid->ComapareArrayWithValue(0.5));
  EXPECT_TRUE(test_grid->ComapareInnerArrayWithValue(0.5));
  EXPECT_TRUE(test_grid->CompareBoundaryValues(0.25));
  EXPECT_TRUE(test_grid->CompareArrays());
}
}  // namespace bdm
