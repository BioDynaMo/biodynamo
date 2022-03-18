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

#ifndef CORE_AGENT_SPHERICAL_AGENT_H_
#define CORE_AGENT_SPHERICAL_AGENT_H_

#include "core/agent/agent.h"
#include "core/container/math_array.h"
#include "core/interaction_force.h"
#include "core/shape.h"
#include "core/util/math.h"

namespace bdm {

class SphericalAgent : public Agent {
  BDM_AGENT_HEADER(SphericalAgent, Agent, 1);

 public:
  SphericalAgent() : diameter_(1.0) {}

  explicit SphericalAgent(real_t diameter) : diameter_(diameter) {}

  explicit SphericalAgent(const Real3& position)
      : position_(position), diameter_(1.0) {}

  virtual ~SphericalAgent() = default;

  Shape GetShape() const override { return Shape::kSphere; }

  real_t GetDiameter() const override { return diameter_; }

  const Real3& GetPosition() const override { return position_; }

  void SetDiameter(real_t diameter) override {
    if (diameter > diameter_) {
      SetPropagateStaticness();
    }
    diameter_ = diameter;
  }

  void SetPosition(const Real3& position) override {
    position_ = position;
    SetPropagateStaticness();
  }

  /// This agent type has an empty implementation for CalculateDisplacement.
  /// Provide an implementation in a derived class if needed.
  Real3 CalculateDisplacement(const InteractionForce* force,
                              real_t squared_radius, real_t dt) override {
    return {0, 0, 0};
  }

  void ApplyDisplacement(const Real3& displacement) override {
    if (displacement[0] == 0 && displacement[1] == 0 && displacement[2] == 0) {
      return;
    }
    position_ += displacement;
    SetPropagateStaticness();
  }

 private:
  /// NB: Use setter and don't assign values directly
  Real3 position_ = {{0, 0, 0}};
  /// NB: Use setter and don't assign values directly
  real_t diameter_ = 0;
};

}  // namespace bdm

#endif  // CORE_AGENT_SPHERICAL_AGENT_H_
