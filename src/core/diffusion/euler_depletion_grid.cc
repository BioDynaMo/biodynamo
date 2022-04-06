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

void EulerDepletionGrid::ApplyDepletion(double dt) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();

  for (size_t s = 0; s < binding_substances_.size(); s++) {
    auto* depleting_concentration =
        rm->GetDiffusionGrid(binding_substances_[s])->GetAllConcentrations();
    if (binding_coefficients_[s] == 0.0) {
      continue;
    } else {
#pragma omp parallel for simd
      for (size_t c = 0; c < total_num_boxes_; c++) {
        c2_[c] = c1_[c] - pre_decay_c1_[c] * binding_coefficients_[s] *
                              depleting_concentration[c] * dt;
      }
      // We swap (==update) after the depletion of each substance
      c1_.swap(c2_);
    }
  }
}

void EulerDepletionGrid::DiffuseWithClosedEdge(double dt) {
  // Get conc vector before diffusion step
  pre_decay_c1_ = c1_;
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithClosedEdge(dt);
  // Deplete
  ApplyDepletion(dt);
}

void EulerDepletionGrid::DiffuseWithOpenEdge(double dt) {
  // Get conc vector before diffusion step
  pre_decay_c1_ = c1_;
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithOpenEdge(dt);
  // Deplete
  ApplyDepletion(dt);
}

}  // namespace bdm
