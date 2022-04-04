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

#include "core/diffusion/euler_depletion_grid.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

namespace bdm {

void EulerDepletionGrid::DiffuseWithClosedEdge(double dt) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();

  // Get conc vector before diffusion step
  auto old_c1 = c1_;
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithClosedEdge(dt);

#pragma omp simd
  for (size_t c = 0; c < total_num_boxes_; c++) {
    for (size_t s = 0; s < binding_substances_.size(); s++) {
      auto* depletesDg = rm->GetDiffusionGrid(binding_substances_[s]);
      c2_[c] = c1_[c] - old_c1[c] * binding_coefficients_[s] *
                            depletesDg->GetAllConcentrations()[c] * dt;
    }
  }
  c1_.swap(c2_);
}

void EulerDepletionGrid::DiffuseWithOpenEdge(double dt) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();

  // Get conc vector before diffusion step
  auto old_c1 = c1_;
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithOpenEdge(dt);

#pragma omp simd
  for (size_t c = 0; c < total_num_boxes_; c++) {
    for (size_t s = 0; s < binding_substances_.size(); s++) {
      auto* depletesDg = rm->GetDiffusionGrid(binding_substances_[s]);
      c2_[c] = c1_[c] - old_c1[c] * binding_coefficients_[s] *
                            depletesDg->GetAllConcentrations()[c] * dt;
    }
  }
  c1_.swap(c2_);
}

}  // namespace bdm
