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

#include "backend.h"
#include "biodynamo.h"
#include "cell.h"
#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "unit/test_util.h"
#include "substance_initializers.h"

namespace bdm {

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();

  using SimObjectTypes = CTList<Cell>;
};

TEST(DiffusionTest, SecretionConcentration) {
  auto set_param = [&](auto* param) {
  param->bound_space_ = true;
  param->min_bound_ = 0;
  param->max_bound_ = 100;
  param->leaking_edges_ = true;
  param->calculate_gradients_ = true;
};

  Simulation<> simulation(TEST_NAME, set_param);
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* param = simulation.GetParam();

  auto cell = rm->New<Cell>();
  cell.SetPosition({50, 50, 50});
  cell.SetDiameter(10);

  ModelInitializer::DefineSubstance(0, "substance1", 0.65, 0,
                                    param->max_bound_ / 2);

  auto& CellPosition = cell->GetPosition();
  DiffusionGrid* dg = rm->GetDiffusionGrid("substance1");

  // initialize grid diffusion
  scheduler->Simulate(1);

  for (int i = 0; i < 100; i++) {
    dg->IncreaseConcentrationBy(CellPosition, 1);
    scheduler->Simulate(1);
  }

  EXPECT_NEAR(dg->GetConcentration({20, 50, 50}),
    0.000297546504698803, abs_error<double>::value);
  EXPECT_NEAR(dg->GetConcentration({25, 50, 50}),
    0.000899639638863217, abs_error<double>::value);
  EXPECT_NEAR(dg->GetConcentration({30, 50, 50}),
    0.00405111093875393, abs_error<double>::value);
  EXPECT_NEAR(dg->GetConcentration({35, 50, 50}),
    0.0101707859583429, abs_error<double>::value);
  EXPECT_NEAR(dg->GetConcentration({40, 50, 50}),
    0.0386124299631847, abs_error<double>::value);

}

}  // end namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
