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
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace chemotaxis_test_ns {

struct TestDiffusionGrid : public DiffusionGrid {
  explicit TestDiffusionGrid(const Double3& normalized_gradient)
      : DiffusionGrid(0, "TestSubstance", 1, 1),
        normalized_gradient_(normalized_gradient) {}

  void GetGradient(const Double3& position, Double3* gradient) const override {
    (*gradient) = normalized_gradient_;
  }

  Double3 normalized_gradient_;
};

TEST(ChemotaxisTest, Run) {
  Simulation simulation(TEST_NAME);

  Cell cell;
  Double3 pos = {10, 10, 10};
  cell.SetPosition(pos);
  cell.SetDiameter(40);

  Double3 normalized_gradient = {1, 2, 3};
  normalized_gradient.Normalize();
  TestDiffusionGrid dgrid(normalized_gradient);

  Chemotaxis ct(&dgrid, 3.14);
  ct.Run(&cell);

  EXPECT_ARR_NEAR(pos + normalized_gradient * 3.14, cell.GetPosition());
}

}  // namespace chemotaxis_test_ns
}  // namespace bdm
