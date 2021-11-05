// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#ifndef UNIT_TEST_UTIL_TEST_AGENT_H_
#define UNIT_TEST_UTIL_TEST_AGENT_H_

#include <set>
#include <string>
#include "core/agent/agent.h"

namespace bdm {

class TestAgent : public Agent {
  BDM_AGENT_HEADER(TestAgent, Agent, 1);

 public:
  TestAgent() {}

  explicit TestAgent(int data) : data_(data) {}

  explicit TestAgent(const Double3& pos) : position_{pos} {}

  virtual ~TestAgent() {}

  Shape GetShape() const override { return Shape::kSphere; };

  std::set<std::string> GetRequiredVisDataMembers() const override {
    return {"diameter_", "position_"};
  }

  void RunDiscretization() override {}

  const Double3& GetPosition() const override { return position_; }

  void SetPosition(const Double3& pos) override { position_ = pos; }

  void ApplyDisplacement(const Double3&) override {}

  Double3 CalculateDisplacement(const InteractionForce* force,
                                double squared_radius, double dt) override {
    return {0, 0, 0};
  }

  double GetDiameter() const override { return diameter_; }
  void SetDiameter(const double diameter) override { diameter_ = diameter; }

  int GetData() const { return data_; }
  void SetData(int data) { data_ = data; }

 protected:
  Double3 position_ = {{0, 0, 0}};
  double diameter_ = 10;
  int data_ = 0;
};

}  // namespace bdm

#endif  // UNIT_TEST_UTIL_TEST_AGENT_H_
