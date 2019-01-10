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

#ifndef DISPLACEMENT_OP_1_H_
#define DISPLACEMENT_OP_1_H_

#include <array>
#include <cmath>
#include <vector>

#include "bound_space_op.h"
#include "math_util.h"
#include "param.h"
#include "simulation.h"

// FIXME
// TODO move to diplacement op cpu

namespace bdm {

template <typename TSimulation = Simulation<>>
class DisplacementOp1 {
 public:
  DisplacementOp1() {
    auto* sim = TSimulation::GetActive();
    auto* grid = sim->GetGrid();

    auto search_radius = grid->GetLargestObjectSize();
    squared_radius_ = search_radius * search_radius;
  }
  ~DisplacementOp1() {}

  template <typename TSimObject>
  void operator()(TSimObject&& sim_object) const {
    auto* sim = TSimulation::GetActive();
    auto* param = sim->GetParam();

    const auto& displacement = sim_object.CalculateDisplacement(squared_radius_);
    sim_object.ApplyDisplacement(displacement);
    if (param->bound_space_) {
      ApplyBoundingBox(&sim_object, param->min_bound_, param->max_bound_);
    }
  }

private:
  double squared_radius_;
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_1_H_
