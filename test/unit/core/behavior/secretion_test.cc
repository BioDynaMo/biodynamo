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

#include "core/behavior/secretion.h"
#include "core/diffusion/euler_grid.h"
#include "core/model_initializer.h"

#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(SecretionTest, Run) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  // Create non-diffusing diffusion grid by setting diffusion coefficient to 0
  ModelInitializer::DefineSubstance(0, "TestSubstance", 0, 0);

  auto* s = new Secretion("TestSubstance", 3.14);

  auto* cell = new Cell();
  Double3 pos = {10, 11, 12};
  cell->SetPosition(pos);
  cell->SetDiameter(40);
  cell->AddBehavior(s);
  rm->AddAgent(cell);

  simulation.Simulate(1);

  auto* dg = rm->GetDiffusionGrid(0);
  auto conc = dg->GetConcentration(pos);

  EXPECT_NEAR(conc, 3.14, 1e-9);
}

}  // namespace bdm
