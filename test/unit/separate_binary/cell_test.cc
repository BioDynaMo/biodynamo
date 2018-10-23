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

#include "cell.h"
#include <typeinfo>
#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "unit/separate_binary/cell_test.h"
#include "unit/test_util.h"

namespace bdm {
namespace cell_test_internal {

TEST(CellTest, TransformCoordinatesGlobalToPolar) {
  TestCell cell;
  cell.TestTransformCoordinatesGlobalToPolar();
}

TEST(CellTest, DivideVolumeRatioPhiTheta) {
  Simulation<> simulation(TEST_NAME);

  TestCell mother;
  mother.SetPosition({5, 6, 7});
  mother.SetTractorForce({0, 0, 0});
  mother.SetDiameter(10);
  mother.UpdateVolume();
  mother.SetAdherence(1.1);
  mother.SetMass(5);
  mother.AddBiologyModule(GrowthModule());
  mother.AddBiologyModule(MovementModule({1, 2, 3}));
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

  // biology modules mother
  EXPECT_EQ(1u, mother.GetAllBiologyModules().size());
  EXPECT_EQ(1u, daughter->GetAllBiologyModules().size());
  if (get_if<GrowthModule>(&(daughter->GetAllBiologyModules()[0])) == nullptr) {
    FAIL() << "Variant type at position 0 is not a GrowthModule";
  }

  EXPECT_EQ(123u, daughter->GetBoxIdx());

  // additional check
  EXPECT_NEAR(5, mother.GetMass() + daughter->GetMass(), kEpsilon);
}

TEST(CellTest, Divide) {
  Simulation<> simulation(TEST_NAME);

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
  Simulation<> simulation(TEST_NAME);

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
  Simulation<> simulation(TEST_NAME);

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
  Simulation<> simulation(TEST_NAME);

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

TEST(CellTest, BiologyModule) {
  Simulation<> simulation(TEST_NAME);

  TestCell cell;
  double diameter = cell.GetDiameter();
  auto position = cell.GetPosition();

  cell.AddBiologyModule(MovementModule({1, 2, 3}));
  cell.AddBiologyModule(GrowthModule());

  cell.RunBiologyModules();

  EXPECT_NEAR(diameter + 0.5, cell.GetDiameter(), abs_error<double>::value);
  EXPECT_NEAR(position[0] + 1, cell.GetPosition()[0], abs_error<double>::value);
  EXPECT_NEAR(position[1] + 2, cell.GetPosition()[1], abs_error<double>::value);
  EXPECT_NEAR(position[2] + 3, cell.GetPosition()[2], abs_error<double>::value);
}

TEST(CellTest, GetBiologyModulesTest) {
  Simulation<> simulation(TEST_NAME);

  // create cell and add bioogy modules
  TestCell cell;
  cell.AddBiologyModule(GrowthModule());
  cell.AddBiologyModule(GrowthModule());
  cell.AddBiologyModule(MovementModule({1, 2, 3}));

  // get all GrowthModules
  auto growth_modules = cell.GetBiologyModules<GrowthModule>();
  EXPECT_EQ(2u, growth_modules.size());

  // get all MovementModules
  auto movement_modules = cell.GetBiologyModules<MovementModule>();
  ASSERT_EQ(1u, movement_modules.size());
  EXPECT_ARR_NEAR(movement_modules[0]->velocity_, {1, 2, 3});
}

TEST(CellTest, BiologyModuleEventHandler) {
  Simulation<> simulation(TEST_NAME);

  TestCell cell;

  cell.AddBiologyModule(MovementModule({1, 2, 3}));
  cell.AddBiologyModule(GrowthModule());

  std::vector<Variant<GrowthModule, MovementModule>> destination;

  auto& src = cell->biology_modules_[cell->kIdx];
  CellDivisionEvent event;
  CopyBiologyModules(event, &src, &destination);
  BiologyModuleEventHandler(event, &src, &destination);

  const auto& bms = cell.GetAllBiologyModules();
  ASSERT_EQ(1u, bms.size());
  EXPECT_TRUE(get_if<GrowthModule>(&bms[0]) != nullptr);
  ASSERT_EQ(1u, destination.size());
  EXPECT_TRUE(get_if<GrowthModule>(&destination[0]) != nullptr);
}

TEST(CellTest, RemoveBiologyModule) {
  Simulation<> simulation(TEST_NAME);

  TestCell cell;

  cell.AddBiologyModule(MovementModule({1, 2, 3}));
  cell.AddBiologyModule(RemoveModule());
  cell.AddBiologyModule(GrowthModule());

  // RemoveModule should remove itself
  cell.RunBiologyModules();

  const auto& bms = cell.GetAllBiologyModules();
  ASSERT_EQ(2u, bms.size());
  EXPECT_TRUE(get_if<MovementModule>(&bms[0]) != nullptr);
  EXPECT_TRUE(get_if<GrowthModule>(&bms[1]) != nullptr);

  cell.AddBiologyModule(RemoveModule());
  ASSERT_EQ(3u, bms.size());
  RemoveModule* to_be_removed =
      const_cast<RemoveModule*>(get_if<RemoveModule>(&bms[2]));
  cell.RemoveBiologyModule(to_be_removed);
  ASSERT_EQ(2u, bms.size());
}

TEST(CellTest, IO) { RunIOTest(); }

}  // namespace cell_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
