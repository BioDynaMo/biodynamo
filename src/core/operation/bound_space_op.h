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

#include "core/param/param.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"

namespace bdm {

inline void ApplyBoundingBox(SimObject* sim_object, double lb, double rb) {
  // Need to create a small distance from the positive edge of each dimension;
  // otherwise it will fall out of the boundary of the simulation space
  double eps = 1e-10;
  auto pos = sim_object->GetPosition();
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
    sim_object->SetPosition(pos);
  }
}

/// Keeps the simulation objects contained within the bounds as defined in
/// param.h
class BoundSpace {
 public:
  BoundSpace() {}
  ~BoundSpace() {}

  void operator()(SimObject* sim_object) const {
    auto* param = Simulation::GetActive()->GetParam();
    if (param->bound_space_) {
      ApplyBoundingBox(sim_object, param->min_bound_, param->max_bound_);
    }
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_BOUND_SPACE_OP_H_
