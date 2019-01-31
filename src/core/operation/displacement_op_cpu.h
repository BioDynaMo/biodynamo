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

#ifndef CORE_OPERATION_DISPLACEMENT_OP_CPU_H_
#define CORE_OPERATION_DISPLACEMENT_OP_CPU_H_

#include <array>
#include <cmath>
#include <limits>
#include <vector>

#include "core/operation/bound_space_op.h"
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/math.h"

namespace bdm {

template <typename TSimulation = Simulation>
class DisplacementOpCpu {
 public:
  DisplacementOpCpu() {}
  ~DisplacementOpCpu() {}

  template <typename TSimObject>
  void operator()(TSimObject&& sim_object) {
    auto* sim = Simulation::GetActive();
    auto* scheduler = sim->GetScheduler();
    auto* param = sim->GetParam();

    // update search radius at beginning of each iteration
    auto current_iteration = scheduler->GetSimulatedSteps();
    if (last_iteration_ != current_iteration) {
      last_iteration_ = current_iteration;

      auto* grid = sim->GetGrid();
      auto search_radius = grid->GetLargestObjectSize();
      squared_radius_ = search_radius * search_radius;
    }

    const auto& displacement =
        sim_object.CalculateDisplacement(squared_radius_);
    sim_object.ApplyDisplacement(displacement);
    if (param->bound_space_) {
      ApplyBoundingBox(&sim_object, param->min_bound_, param->max_bound_);
    }
  }

 private:
  double squared_radius_;
  uint64_t last_iteration_ = std::numeric_limits<uint64_t>::max();
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_CPU_H_
