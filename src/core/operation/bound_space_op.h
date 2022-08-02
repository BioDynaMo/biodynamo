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

#ifndef CORE_OPERATION_BOUND_SPACE_OP_H_
#define CORE_OPERATION_BOUND_SPACE_OP_H_

#include "core/agent/agent.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/simulation_space.h"

namespace bdm {

inline void ApplyBoundingBox(Agent* agent, Param::BoundSpaceMode mode) {
  // Need to create a small distance from the positive edge of each dimension;
  // otherwise it will fall out of the boundary of the simulation space
  real_t eps = 1e-10;
  auto pos = agent->GetPosition();
  auto* space = Simulation::GetActive()->GetSimulationSpace();
  const auto& whole_space = space->GetWholeSpace();
  if (mode == Param::BoundSpaceMode::kClosed) {
    bool updated = false;
    for (int i = 0; i < 3; i++) {
      auto lb = whole_space[2 * i];
      auto rb = whole_space[2 * i + 1];
      if (pos[i] < lb) {
        pos[i] = lb;
        updated = true;
      } else if (pos[i] >= rb) {
        pos[i] = rb - eps;
        updated = true;
      }
    }
    if (updated) {
      agent->SetPosition(pos);
    }
  } else if (mode == Param::BoundSpaceMode::kTorus) {
    for (int i = 0; i < 3; i++) {
      auto& el = pos[i];
      auto lb = whole_space[2 * i];
      auto rb = whole_space[2 * i + 1];
      auto length = rb - lb;
      if (el < lb) {
        auto d = std::abs(lb - el);
        if (d > length) {
          d = std::fmod(d, length);
        }
        el = rb - d;
      } else if (el > rb) {
        auto d = std::abs(el - rb);
        if (d > length) {
          d = std::fmod(d, length);
        }
        el = lb + d;
      }
    }
    agent->SetPosition(pos);
  }
}

/// Keeps the agents contained within the bounds as defined in
/// param.h
struct BoundSpace : public AgentOperationImpl {
  BDM_OP_HEADER(BoundSpace);

  void operator()(Agent* agent) override {
    auto* param = Simulation::GetActive()->GetParam();
    if (param->bound_space) {
      ApplyBoundingBox(agent, param->bound_space);
    }
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_BOUND_SPACE_OP_H_
