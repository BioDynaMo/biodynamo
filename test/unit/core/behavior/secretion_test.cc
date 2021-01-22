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
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {

struct TestDiffusionGrid : public DiffusionGrid {
  TestDiffusionGrid() : DiffusionGrid(0, "TestSubstance", 1, 1) {}

  void ChangeConcentrationBy(const Double3& position, double amount) override {
    EXPECT_ARR_NEAR({10, 11, 12}, position);
    EXPECT_NEAR(3.14, amount, abs_error<double>::value);
    called = true;
  }

  bool called = false;
};

TEST(SecretionTest, Run) {
  Simulation simulation(TEST_NAME);

  Cell cell;
  Double3 pos = {10, 11, 12};
  cell.SetPosition(pos);
  cell.SetDiameter(40);

  TestDiffusionGrid dgrid;

  Secretion s(&dgrid, 3.14);
  s.Run(&cell);

  EXPECT_TRUE(dgrid.called);
}

}  // namespace bdm
