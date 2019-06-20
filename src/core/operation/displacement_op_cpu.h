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

#include "core/grid.h"
#include "core/operation/bound_space_op.h"
#include "core/param/param.h"
#include "core/scheduler.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/util/math.h"

namespace bdm {

class DisplacementOpCpu {
 public:
  DisplacementOpCpu() {}
  ~DisplacementOpCpu() {}

  void operator()(SimObject* sim_object) {
    auto* sim = Simulation::GetActive();
    auto* scheduler = sim->GetScheduler();
    auto* param = sim->GetParam();

    if (!sim_object->RunDisplacement()) {
      return;
    }

    // update search radius and delta_time_ at beginning of each iteration
    auto current_iteration = scheduler->GetSimulatedSteps();
    if (last_iteration_ != current_iteration) {
      last_iteration_ = current_iteration;

      auto* grid = sim->GetGrid();
      auto search_radius = grid->GetLargestObjectSize();
      squared_radius_ = search_radius * search_radius;
      auto current_time =
          (current_iteration + 1) * param->simulation_time_step_;
      delta_time_ = current_time - last_time_run_;
      last_time_run_ = current_time;
    }

    const auto& displacement =
        sim_object->CalculateDisplacement(squared_radius_, delta_time_);
    sim_object->ApplyDisplacement(displacement);
    if (param->bound_space_) {
      ApplyBoundingBox(sim_object, param->min_bound_, param->max_bound_);
    }
  }

 private:
  double squared_radius_ = 0;
  double last_time_run_ = 0;
  double delta_time_ = 0;
  uint64_t last_iteration_ = std::numeric_limits<uint64_t>::max();
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_CPU_H_
