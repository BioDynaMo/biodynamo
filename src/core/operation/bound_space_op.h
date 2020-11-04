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

#ifndef CORE_OPERATION_BOUND_SPACE_OP_H_
#define CORE_OPERATION_BOUND_SPACE_OP_H_

#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/agent/agent.h"
#include "core/simulation.h"

namespace bdm {

inline void ApplyBoundingBox(Agent* agent, double lb, double rb) {
  // Need to create a small distance from the positive edge of each dimension;
  // otherwise it will fall out of the boundary of the simulation space
  double eps = 1e-10;
  auto pos = agent->GetPosition();
  bool updated = false;
  for (int i = 0; i < 3; i++) {
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
}

/// Keeps the agents contained within the bounds as defined in
/// param.h
struct BoundSpace : public AgentOperationImpl {
  BDM_OP_HEADER(BoundSpace);

  void operator()(Agent* agent) override {
    auto* param = Simulation::GetActive()->GetParam();
    if (param->bound_space_) {
      ApplyBoundingBox(agent, param->min_bound_, param->max_bound_);
    }
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_BOUND_SPACE_OP_H_
