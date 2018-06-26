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

#ifndef DISPLACEMENT_OP_CPU_H_
#define DISPLACEMENT_OP_CPU_H_

#include <array>
#include <cmath>
#include <vector>

#include "bound_space_op.h"
#include "math_util.h"
#include "param.h"
#include "simulation.h"

namespace bdm {

template <typename TSimulation = Simulation<>>
class DisplacementOpCpu {
 public:
  DisplacementOpCpu() {}
  ~DisplacementOpCpu() {}

  template <typename TContainer>
  void operator()(TContainer* sim_objects, uint16_t type_idx) const {
    std::vector<array<double, 3>> sim_object_movements;
    sim_object_movements.reserve(sim_objects->size());

    auto* sim = TSimulation::GetActive();
    auto* grid = sim->GetGrid();

    auto search_radius = grid->GetLargestObjectSize();
    double squared_radius = search_radius * search_radius;

#pragma omp parallel for shared(grid) firstprivate(squared_radius)
    for (size_t i = 0; i < sim_objects->size(); i++) {
      sim_object_movements[i] =
          (*sim_objects)[i].CalculateDisplacement(grid, squared_radius);
    }

    // Set new positions after all updates have been calculated
    // otherwise some sim_objects would see neighbors with already updated
    // positions
    // which would lead to inconsistencies
    // FIXME there are still inconsistencies if there are more than one
    // simulation
    //  object types!
    auto* param = sim->GetParam();
#pragma omp parallel for
    for (size_t i = 0; i < sim_objects->size(); i++) {
      auto&& sim_object = (*sim_objects)[i];
      sim_object.ApplyDisplacement(sim_object_movements[i]);
      if (param->bound_space_) {
        ApplyBoundingBox(&sim_object, param->min_bound_, param->max_bound_);
        grid->SetDimensionThresholds(param->min_bound_, param->max_bound_);
      }
    }
  }
};

}  // namespace bdm

#endif  // DISPLACEMENT_OP_CPU_H_
