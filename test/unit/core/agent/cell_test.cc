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

#include "unit/core/agent/cell_test.h"
#include <gtest/gtest.h>
#include <typeinfo>
#include "core/agent/cell.h"
#include "unit/core/agent/agent_test.h"
#include "unit/test_util/test_util.h"

namespace bdm {
namespace cell_test_internal {

using agent_test_internal::Growth;
using agent_test_internal::Movement;

TEST(CellTest, TransformCoordinatesGlobalToPolar) {
  Simulation simulation(TEST_NAME);

  TestCell cell;
  cell.TestTransformCoordinatesGlobalToPolar();
}

TEST(CellTest, DivideVolumeRatioPhiTheta) {
  Simulation simulation(TEST_NAME);

  TestCell mother;
  mother.SetPosition({5, 6, 7});
  mother.SetTractorForce({0, 0, 0});
  mother.SetDiameter(10);
  mother.UpdateVolume();
  mother.SetAdherence(1.1);
  mother.SetMass(5);
  mother.AddBehavior(new Growth());
  mother.AddBehavior(new Movement({1, 2, 3}));
  mother.SetBoxIdx(123);

  auto daughter = mother.Divide(0.75, 0.12, 0.34);

  const double kEpsilon = abs_error<double>::value;

  // verify mother data members
  EXPECT_NEAR(4.9244246147707642, mother.GetPosition()[0], kEpsilon);
  EXPECT_NEAR(5.9732661991724063, mother.GetPosition()[1], kEpsilon);
  EXPECT_NEAR(6.335172788490714, mother.GetPosition()[2], kEpsilon);

  EXPECT_NEAR(0, mother.GetTractorForce()[0], kEpsilon);
  EXPECT_NEAR(0, mother.GetTractorForce()[1], kEpsilon);
  EXPECT_NEAR(0, mother.GetTractorForce()[2], kEpsilon);

  EXPECT_NEAR(8.2982653336624335, mother.GetDiameter(), kEpsilon);
  // differs slightly from the value in branch validation due to more precise
  // value of PI
  EXPECT_NEAR(299.19930034188491, mother.GetVolume(), kEpsilon);
  EXPECT_NEAR(1.1, mother.GetAdherence(), kEpsilon);
  EXPECT_NEAR(2.8571428571428563, mother.GetMass(), kEpsilon);

  EXPECT_EQ(123u, mother.GetBoxIdx());

  // verify daughter data members
  EXPECT_NEAR(5.1007671803056471, daughter->GetPosition()[0], kEpsilon);
  EXPECT_NEAR(6.0356450677701252, daughter->GetPosition()[1], kEpsilon);
  EXPECT_NEAR(7.8864362820123803, daughter->GetPosition()[2], kEpsilon);

  EXPECT_NEAR(0, daughter->GetTractorForce()[0], kEpsilon);
  EXPECT_NEAR(0, daughter->GetTractorForce()[1], kEpsilon);
  EXPECT_NEAR(0, daughter->GetTractorForce()[2], kEpsilon);

  EXPECT_NEAR(7.5394744112915388, daughter->GetDiameter(), kEpsilon);
  // differs slightly from the value in branch validation due to more precise
  // value of PI
  EXPECT_NEAR(224.39947525641387, daughter->GetVolume(), kEpsilon);
  EXPECT_NEAR(1.1, daughter->GetAdherence(), kEpsilon);
  EXPECT_NEAR(2.1428571428571437, daughter->GetMass(), kEpsilon);

  // behaviors mother
  EXPECT_EQ(1u, mother.GetAllBehaviors().size());
  EXPECT_EQ(1u, daughter->GetAllBehaviors().size());
  if (dynamic_cast<Growth*>(daughter->GetAllBehaviors()[0]) == nullptr) {
    FAIL() << "Variant type at position 0 is not a Growth";
  }

  EXPECT_EQ(123u, daughter->GetBoxIdx());

  // additional check
  EXPECT_NEAR(5, mother.GetMass() + daughter->GetMass(), kEpsilon);
}

TEST(CellTest, Divide) {
  Simulation simulation(TEST_NAME);

  TestCell cell;

#pragma omp parallel
  simulation.GetRandom()->SetSeed(42);

  cell.capture_input_parameters_ = true;
  cell.Divide();

  EXPECT_NEAR(cell.captured_volume_ratio_, 1.0, 0.1);             // (0.9 - 1.1)
  EXPECT_NEAR(cell.captured_theta_, Math::kPi, Math::kPi);        // (0 - 2 PI)
  EXPECT_NEAR(cell.captured_phi_, Math::kPi / 2, Math::kPi / 2);  // (0 - PI)
}

TEST(CellTest, DivideVolumeRatio) {
  Simulation simulation(TEST_NAME);

#pragma omp parallel
  simulation.GetRandom()->SetSeed(42);

  TestCell cell;
  cell.capture_input_parameters_ = true;
  cell.Divide(0.59);

  const double kEpsilon = abs_error<double>::value;
  EXPECT_NEAR(cell.captured_volume_ratio_, 0.59, kEpsilon);
  EXPECT_NEAR(cell.captured_theta_, Math::kPi, Math::kPi);        // (0 - 2 PI)
  EXPECT_NEAR(cell.captured_phi_, Math::kPi / 2, Math::kPi / 2);  // (0 - PI)
}

TEST(CellTest, DivideAxis) {
  Simulation simulation(TEST_NAME);

#pragma omp parallel
  simulation.GetRandom()->SetSeed(42);

  TestCell cell;
  cell.SetPosition({1, 2, 3});

  cell.capture_input_parameters_ = true;
  cell.Divide({9, 8, 7});

  const double kEpsilon = abs_error<double>::value;
  EXPECT_NEAR(cell.captured_volume_ratio_, 1.0, 0.1);  // (0.9 - 1.1)
  EXPECT_NEAR(cell.captured_phi_, 1.0442265974045177, kEpsilon);
  EXPECT_NEAR(cell.captured_theta_, 0.72664234068172562, kEpsilon);
}

TEST(CellTest, DivideVolumeRatioAxis) {
  Simulation simulation(TEST_NAME);

#pragma omp parallel
  simulation.GetRandom()->SetSeed(42);

  TestCell cell;
  cell.SetPosition({1, 2, 3});

  cell.capture_input_parameters_ = true;
  cell.Divide(0.456, {9, 8, 7});

  const double kEpsilon = abs_error<double>::value;
  EXPECT_NEAR(cell.captured_volume_ratio_, 0.456, kEpsilon);
  EXPECT_NEAR(cell.captured_phi_, 1.0442265974045177, kEpsilon);
  EXPECT_NEAR(cell.captured_theta_, 0.72664234068172562, kEpsilon);
}

#ifdef USE_DICT
TEST(CellTest, IO) { RunIOTest(); }
#endif  // USE_DICT

}  // namespace cell_test_internal
}  // namespace bdm
