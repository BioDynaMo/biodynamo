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

#ifndef UNIT_SEPARATE_BINARY_CELL_TEST_H_
#define UNIT_SEPARATE_BINARY_CELL_TEST_H_

#include <vector>

#include "biology_module_util.h"
#include "cell.h"
#include "compile_time_param.h"
#include "event/cell_division_event.h"
#include "gtest/gtest.h"
#include "io_util.h"
#include "unit/test_util.h"

#define ROOTFILE "bdmFile.root"

namespace bdm {
namespace cell_test_internal {

struct GrowthModule : public BaseBiologyModule {
  double growth_rate_ = 0.5;
  GrowthModule() : BaseBiologyModule(CellDivisionEvent::kEventId) {}

  // Ctor for any event
  template <typename TEvent, typename TBm>
  GrowthModule(const TEvent& event, TBm* other, uint64_t new_oid = 0) {
    growth_rate_ = other->growth_rate_;
  }

  // empty event handler (exising biology module won't be modified on any event)
  template <typename TEvent, typename... TBms>
  void EventHandler(const TEvent&, TBms*...) {}

  template <typename T>
  void Run(T* t) {
    t->SetDiameter(t->GetDiameter() + growth_rate_);
  }

  BDM_CLASS_DEF_NV(GrowthModule, 1);
};

struct MovementModule {
  std::array<double, 3> velocity_;

  MovementModule() : velocity_({{0, 0, 0}}) {}
  explicit MovementModule(const std::array<double, 3>& velocity)
      : velocity_(velocity) {}

  // Ctor for any event
  template <typename TEvent, typename TBm>
  MovementModule(const TEvent& event, TBm* other, uint64_t new_oid = 0) {
    velocity_ = other->velocity_;
  }

  // empty event handler (exising biology module won't be modified on any event)
  template <typename TEvent, typename... TBms>
  void EventHandler(const TEvent&, TBms*...) {}

  template <typename T>
  void Run(T* t) {
    const auto& position = t->GetPosition();
    t->SetPosition(Math::Add(position, velocity_));
  }

  bool Copy(EventId event) const { return false; }
  bool Remove(EventId event) const {
    return event == CellDivisionEvent::kEventId;
  }
  BDM_CLASS_DEF_NV(MovementModule, 1);
};

/// This biology module removes itself the first time it is executed
struct RemoveModule : public BaseBiologyModule {
  RemoveModule() {}

  // Ctor for any event
  template <typename TEvent, typename TBm>
  RemoveModule(const TEvent& event, TBm* other, uint64_t new_oid = 0) {}

  template <typename TSimObject>
  void Run(TSimObject* sim_object) {
    sim_object->RemoveBiologyModule(this);
  }

  BDM_CLASS_DEF_NV(RemoveModule, 1);
};

/// Class used to get access to protected members
BDM_SIM_OBJECT(TestCell, Cell) {
  BDM_SIM_OBJECT_HEADER(TestCell, Cell, 1, placeholder_);

 public:
  TestCellExt() {}

  // Ctor for CellDivisionEvent
  TestCellExt(CellDivisionEvent event, TestCellExt * mother,
              uint64_t new_oid = 0)
      : Base(event, mother, new_oid) {
    if (mother->capture_input_parameters_) {
      mother->captured_volume_ratio_ = event.volume_ratio_;
      mother->captured_phi_ = event.phi_;
      mother->captured_theta_ = event.theta_;
    }
  }

  template <typename TDaughter>
  void EventHandler(CellDivisionEvent event, TDaughter * daughter) {
    Base::EventHandler(event, daughter);
  }

  void TestTransformCoordinatesGlobalToPolar() {
    std::array<double, 3> coord = {1, 2, 3};
    Base::SetPosition({9, 8, 7});
    auto result = Base::TransformCoordinatesGlobalToPolar(coord);

    EXPECT_NEAR(10.770329614269007, result[0], abs_error<double>::value);
    EXPECT_NEAR(1.9513027039072615, result[1], abs_error<double>::value);
    EXPECT_NEAR(-2.4980915447965089, result[2], abs_error<double>::value);
  }

  const auto& GetAllBiologyModules() const {
    return Base::biology_modules_[kIdx];
  }

  bool capture_input_parameters_ = false;
  double captured_volume_ratio_ = 0.0;
  double captured_phi_ = 0.0;
  double captured_theta_ = 0.0;

  vec<bool> placeholder_;  // BDM_SIM_OBJECT_HEADER needs at least one member
  FRIEND_TEST(CellTest, DivideVolumeRatioPhiTheta);
};

}  // namespace cell_test_internal

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();

  using SimObjectTypes = CTList<cell_test_internal::TestCell>;
  BDM_DEFAULT_CTPARAM_FOR(cell_test_internal::TestCell) {
    using BiologyModules = CTList<cell_test_internal::GrowthModule,
                                  cell_test_internal::MovementModule,
                                  cell_test_internal::RemoveModule>;
  };
};

namespace cell_test_internal {

inline void RunIOTest() {
  remove(ROOTFILE);

  TestCell cell;
  cell.SetPosition({5, 6, 7});
  cell.SetTractorForce({7, 4, 1});
  cell.SetDiameter(12);
  cell.UpdateVolume();
  cell.SetAdherence(1.1);
  cell.SetMass(5);
  cell.AddBiologyModule(GrowthModule());
  cell.AddBiologyModule(MovementModule({1, 2, 3}));
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
  EXPECT_TRUE(get_if<GrowthModule>(&restored_cell->GetAllBiologyModules()[0]) !=
              nullptr);
  EXPECT_NEAR(0.5,
              get_if<GrowthModule>(&restored_cell->GetAllBiologyModules()[0])
                  ->growth_rate_,
              kEpsilon);
  EXPECT_TRUE(get_if<MovementModule>(
                  &restored_cell->GetAllBiologyModules()[1]) != nullptr);

  EXPECT_EQ(123u, restored_cell->GetBoxIdx());

  delete restored_cell;
  // delete root file
  remove(ROOTFILE);
}

}  // namespace cell_test_internal
}  // namespace bdm

#endif  // UNIT_SEPARATE_BINARY_CELL_TEST_H_
