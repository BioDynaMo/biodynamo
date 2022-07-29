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

#ifndef CORE_OPERATION_DIFFUSION_OP_H_
#define CORE_OPERATION_DIFFUSION_OP_H_

#include <string>
#include <utility>
#include <vector>

#include "core/container/inline_vector.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/environment/environment.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/log.h"

namespace bdm {

/// A class that sets up diffusion grids of the substances in this simulation
class DiffusionOp : public StandaloneOperationImpl {
 protected:
  /// Last time when the operation was executed
  real_t last_time_run_ = 0.0;
  /// Timestep that is useded for `Diffuse(delta_t)` and computed from this and
  /// the last time the grid was updated.
  real_t delta_t_ = 0.0;

 public:
  BDM_OP_HEADER(DiffusionOp);

  void operator()() override {
    // Get active simulation and related pointers
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    auto* env = sim->GetEnvironment();
    auto* param = sim->GetParam();

    // Compute the passed time to update the diffusion grid accordingly.
    real_t current_time = sim->GetScheduler()->GetSimulatedTime();
    delta_t_ = current_time - last_time_run_;
    last_time_run_ = current_time;

    // Avoid computation if delta_t_ is zero
    if (delta_t_ == 0.0) {
      return;
    }

    rm->ForEachDiffusionGrid([&](DiffusionGrid* dgrid) {
      dgrid->Update();
      dgrid->Diffuse(delta_t_);
      if (param->calculate_gradients) {
        dgrid->CalculateGradient();
      }
    });
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_DIFFUSION_OP_H_
