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

#ifndef UNIT_TEST_SIM_OBJECT_H_
#define UNIT_TEST_SIM_OBJECT_H_

#include "simulation_object_util.h"

namespace bdm {

BDM_SIM_OBJECT(TestSimObject, SimulationObject) {
  BDM_SIM_OBJECT_HEADER(TestSimObject, SimulationObject, 1, foo_);

 public:
  static std::set<std::string> GetRequiredVisDataMembers() {
    return {"diameter_", "position_"};
  }

  static constexpr Shape GetShape() { return Shape::kSphere; }

  TestSimObjectExt() {}

  template <typename TEvent, typename TOther>
  TestSimObjectExt(const TEvent& event, TOther* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}

  template <typename TEvent, typename TDaughter>
  void EventHandler(const TEvent& event, TDaughter* daughter) {
    Base::EventHandler(event, daughter);
  }

  std::array<double, 3> GetPosition() const { return {0, 0, 0}; }

  void SetPosition(const std::array<double, 3>&) {}

  void ApplyDisplacement(const std::array<double, 3>&) {}

  std::array<double, 3> CalculateDisplacement(double squared_radius) {
    return {0, 0, 0};
  }

  void SetBoxIdx(uint64_t) {}

  double GetDiameter() const { return 3.14; }

  vec<int> foo_;
};

}  // namespace bdm

#endif  // UNIT_TEST_SIM_OBJECT_H_
