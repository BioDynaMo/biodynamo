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

#include "core/behavior/chemotaxis.h"
#include "core/behavior/secretion.h"
#include "core/diffusion/euler_grid.h"
#include "core/model_initializer.h"

#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(ChemotaxisTest, Run) {
  Simulation simulation(TEST_NAME);
  auto* rm = simulation.GetResourceManager();
  ModelInitializer::DefineSubstance(0, "TestSubstance", 0.5, 0, 3);

  Double3 normalized_gradient = {1, 2, 3};
  normalized_gradient.Normalize();
  auto* s = new Secretion("TestSubstance", 3.14);
  auto* ct = new Chemotaxis("TestSubstance", 3.14);

  auto* scell = new Cell();
  Double3 pos = {0, 0, 0};
  scell->SetPosition(pos);
  scell->SetDiameter(40);
  scell->AddBehavior(s);
  rm->AddAgent(scell);

  auto* cell = new Cell();
  pos = {10, 10, 10};
  cell->SetPosition(pos);
  cell->SetDiameter(40);
  cell->AddBehavior(ct);
  rm->AddAgent(cell);

  simulation.Simulate(1);

  auto final_pos = rm->GetAgent(AgentUid(1))->GetPosition();
  auto movement = final_pos - pos;

  EXPECT_TRUE(movement[0] > abs_error<double>::value);
  EXPECT_TRUE(movement[1] > abs_error<double>::value);
  EXPECT_TRUE(movement[2] > abs_error<double>::value);
}

}  // namespace bdm
