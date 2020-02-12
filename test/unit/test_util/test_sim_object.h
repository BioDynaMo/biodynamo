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

#ifndef UNIT_TEST_UTIL_TEST_SIM_OBJECT_H_
#define UNIT_TEST_UTIL_TEST_SIM_OBJECT_H_

#include <set>
#include <string>
#include "core/sim_object/sim_object.h"

namespace bdm {

class TestSimObject : public SimObject {
  BDM_SIM_OBJECT_HEADER(TestSimObject, SimObject, 1, position_, diameter_);

 public:
  TestSimObject() {}

  explicit TestSimObject(int data) : data_(data) {}

  explicit TestSimObject(const Double3& pos) : position_{pos} {}

  TestSimObject(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {}

  virtual ~TestSimObject() {}

  Shape GetShape() const override { return Shape::kSphere; };

  std::set<std::string> GetRequiredVisDataMembers() const override {
    return {"diameter_", "position_"};
  }

  void RunDiscretization() override {}

  const Double3& GetPosition() const override { return position_; }

  void SetPosition(const Double3& pos) override { position_ = pos; }

  void ApplyDisplacement(const Double3&) override {}

  Double3 CalculateDisplacement(double squared_radius, double dt) override {
    return {0, 0, 0};
  }

  double GetDiameter() const override { return diameter_; }
  void SetDiameter(const double diameter) override { diameter_ = diameter; }

  int GetData() const { return data_; }
  void SetData(double data) { data_ = data; }

 protected:
  Double3 position_ = {{0, 0, 0}};
  double diameter_ = 10;
  int data_ = 0;
};

}  // namespace bdm

#endif  // UNIT_TEST_UTIL_TEST_SIM_OBJECT_H_
