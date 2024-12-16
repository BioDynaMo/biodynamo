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

void EulerDepletionGrid::ApplyDepletion(real_t dt) {
  auto* sim = Simulation::GetActive();
  const auto* rm = sim->GetResourceManager();

  // The explicit scheme computes the new concentarion c2 based on c1. Efficient
  // updates are ensured by swapping pointers at each step. Here, however, we
  // want to continue to use c1 for the next step. Thus, we swap pointers here
  // (and again after the depletion). This is necessary because ApplyDepletion
  // is called after the diffusion of the EulerGrid (swaps pointer at the end).
  std::swap(c1_, c2_);

  for (size_t s = 0; s < binding_substances_.size(); s++) {
    if (binding_coefficients_[s] == 0.0) {
      // If the binding coefficient is zero, we do not need to apply the
      // depletion.
      continue;
    }

    size_t depleting_boxes =
        rm->GetDiffusionGrid(binding_substances_[s])->GetNumBoxes();

    if (depleting_boxes != GetNumBoxes()) {
      Log::Fatal(
          "EulerDepletionGrid::ApplyDepletion()",
          "The number of voxels of the depleting diffusion grid ",
          rm->GetDiffusionGrid(binding_substances_[s])->GetContinuumName(),
          " differs from that of the depleted one (", GetContinuumName(),
          "). Check the resolution.");
    } else {
      auto* depleting_concentration =
          rm->GetDiffusionGrid(binding_substances_[s])->GetAllConcentrations();
#pragma omp parallel for simd
      for (size_t c = 0; c < total_num_boxes_; c++) {
        c2_[c] -=
            c1_[c] * binding_coefficients_[s] * depleting_concentration[c] * dt;
      }
    }
  }
  // See comment above.
  std::swap(c1_, c2_);
}

void EulerDepletionGrid::DiffuseWithClosedEdge(real_t dt) {
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithClosedEdge(dt);

  ApplyDepletion(dt);
}

void EulerDepletionGrid::DiffuseWithOpenEdge(real_t dt) {
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithOpenEdge(dt);

  ApplyDepletion(dt);
}

void EulerDepletionGrid::DiffuseWithDirichlet(real_t dt) {
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithDirichlet(dt);

  ApplyDepletion(dt);
}

void EulerDepletionGrid::DiffuseWithNeumann(real_t dt) {
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithNeumann(dt);

  ApplyDepletion(dt);
}

void EulerDepletionGrid::DiffuseWithPeriodic(real_t dt) {
  // Update concentration without depletion (c1 is modified)
  EulerGrid::DiffuseWithPeriodic(dt);

  ApplyDepletion(dt);
}

}  // namespace bdm
