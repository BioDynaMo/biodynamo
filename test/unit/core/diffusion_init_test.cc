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

#include "core/agent/cell.h"
#include "core/diffusion_grid.h"
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
    param->bound_space = true;
    param->min_bound = 0;
    param->max_bound = 250;
  };
  Simulation simulation(TEST_NAME, set_param);

  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();

  // Create one cell at a random position
  auto construct = [](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound, param->max_bound, 1,
                                      construct);

  // Define the substances in our simulation
  ModelInitializer::DefineSubstance(kSubstance, "Substance", 0.5, 0.1, 26);

  // Initialize the substance according to a GaussianBand along the x-axis
  ModelInitializer::InitializeSubstance(kSubstance,
                                        GaussianBand(125, 50, Axis::kXAxis));

  simulation.GetEnvironment()->Update();

  int lbound = param->min_bound;
  int rbound = param->max_bound;
  auto* dgrid = rm->GetDiffusionGrid(0);

  // Create data structures, whose size depend on the grid dimensions
  dgrid->Initialize({lbound, rbound, lbound, rbound, lbound, rbound});
  // Initialize data structures with user-defined values
  dgrid->RunInitializers();

  std::array<uint32_t, 3> a = {0, 0, 0};
  std::array<uint32_t, 3> b = {25, 0, 0};
  std::array<uint32_t, 3> c = {13, 0, 0};
  std::array<uint32_t, 3> d = {0, 13, 0};
  std::array<uint32_t, 3> e = {25, 0, 13};
  std::array<uint32_t, 3> f = {13, 13, 13};

  auto eps = abs_error<double>::value;
  auto conc = dgrid->GetAllConcentrations();

  EXPECT_NEAR(ROOT::Math::normal_pdf(0, 50, 125), conc[dgrid->GetBoxIndex(a)],
              eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(250, 50, 125), conc[dgrid->GetBoxIndex(b)],
              eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(130, 50, 125), conc[dgrid->GetBoxIndex(c)],
              eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(0, 50, 125), conc[dgrid->GetBoxIndex(d)],
              eps);
  // Should be symmetric, so the two ends should have the same value
  EXPECT_NEAR(ROOT::Math::normal_pdf(0, 50, 125), conc[dgrid->GetBoxIndex(e)],
              eps);
  EXPECT_NEAR(ROOT::Math::normal_pdf(130, 50, 125), conc[dgrid->GetBoxIndex(f)],
              eps);
}

}  // namespace bdm
