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

#ifndef UNIT_CORE_SIM_OBJECT_CELL_TEST_H_
#define UNIT_CORE_SIM_OBJECT_CELL_TEST_H_

#include <vector>

#include "core/event/cell_division_event.h"
#include "core/sim_object/cell.h"
#include "core/util/io.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"
#include "unit/core/sim_object/sim_object_test.h"


#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace cell_test_internal {

/// Class used to get access to protected members
class TestCell : public Cell {
  BDM_SIM_OBJECT_HEADER(TestCell, Cell, 1, placeholder_);

 public:
  TestCell() {}

  virtual ~TestCell() {}

  TestCell(const Event& event, SimObject* mother, uint64_t new_oid = 0) : Base(event, mother, new_oid) {
    const CellDivisionEvent* cdevent = dynamic_cast<const CellDivisionEvent*>(&event);
    TestCell* mother_cell = dynamic_cast<TestCell*>(mother);
    if(cdevent && mother_cell && mother_cell->capture_input_parameters_) {
      mother_cell->captured_volume_ratio_ = cdevent->volume_ratio_;
      mother_cell->captured_phi_ = cdevent->phi_;
      mother_cell->captured_theta_ = cdevent->theta_;
    }
  }

  void TestTransformCoordinatesGlobalToPolar() {
    std::array<double, 3> coord = {1, 2, 3};
    Base::SetPosition({9, 8, 7});
    auto result = Base::TransformCoordinatesGlobalToPolar(coord);

    EXPECT_NEAR(10.770329614269007, result[0], abs_error<double>::value);
    EXPECT_NEAR(1.9513027039072615, result[1], abs_error<double>::value);
    EXPECT_NEAR(-2.4980915447965089, result[2], abs_error<double>::value);
  }

  bool capture_input_parameters_ = false;
  double captured_volume_ratio_ = 0.0;
  double captured_phi_ = 0.0;
  double captured_theta_ = 0.0;

  bool placeholder_;  // BDM_SIM_OBJECT_HEADER needs at least one member
  FRIEND_TEST(CellTest, DivideVolumeRatioPhiTheta);
};

}  // namespace cell_test_internal

namespace cell_test_internal {

inline void RunIOTest() {
  using GrowthModule = sim_object_test_internal::GrowthModule;
  using MovementModule = sim_object_test_internal::MovementModule;
  remove(ROOTFILE);

  TestCell cell;
  cell.SetPosition({5, 6, 7});
  cell.SetTractorForce({7, 4, 1});
  cell.SetDiameter(12);
  cell.UpdateVolume();
  cell.SetAdherence(1.1);
  cell.SetMass(5);
  cell.AddBiologyModule(new GrowthModule());
  cell.AddBiologyModule(new MovementModule({1, 2, 3}));
  cell.SetBoxIdx(123);

  // write to root file
  WritePersistentObject(ROOTFILE, "cell", cell, "new");

  // read back
  TestCell* restored_cell = nullptr;
  GetPersistentObject(ROOTFILE, "cell", restored_cell);

  // validate
  const double kEpsilon = abs_error<double>::value;
  EXPECT_NEAR(5, restored_cell->GetPosition()[0], kEpsilon);
  EXPECT_NEAR(6, restored_cell->GetPosition()[1], kEpsilon);
  EXPECT_NEAR(7, restored_cell->GetPosition()[2], kEpsilon);

  EXPECT_NEAR(7, restored_cell->GetTractorForce()[0], kEpsilon);
  EXPECT_NEAR(4, restored_cell->GetTractorForce()[1], kEpsilon);
  EXPECT_NEAR(1, restored_cell->GetTractorForce()[2], kEpsilon);

  EXPECT_NEAR(12, restored_cell->GetDiameter(), kEpsilon);
  // differs slightly from the value in branch validation due to more precise
  // value of PI
  EXPECT_NEAR(cell.GetVolume(), restored_cell->GetVolume(), kEpsilon);
  EXPECT_NEAR(1.1, restored_cell->GetAdherence(), kEpsilon);
  EXPECT_NEAR(5, restored_cell->GetMass(), kEpsilon);

  EXPECT_EQ(2u, restored_cell->GetAllBiologyModules().size());
  EXPECT_TRUE(dynamic_cast<GrowthModule*>(restored_cell->GetAllBiologyModules()[0]) !=
              nullptr);
  EXPECT_NEAR(0.5,
              dynamic_cast<GrowthModule*>(restored_cell->GetAllBiologyModules()[0])
                  ->growth_rate_,
              kEpsilon);
  EXPECT_TRUE(dynamic_cast<MovementModule*>(
                  restored_cell->GetAllBiologyModules()[1]) != nullptr);

  EXPECT_EQ(123u, restored_cell->GetBoxIdx());

  delete restored_cell;
  // delete root file
  remove(ROOTFILE);
}

}  // namespace cell_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_SIM_OBJECT_CELL_TEST_H_
