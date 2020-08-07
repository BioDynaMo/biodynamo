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

#ifndef CORE_OPERATION_DIFFUSION_OP_H_
#define CORE_OPERATION_DIFFUSION_OP_H_

#include <string>
#include <utility>
#include <vector>

#include "core/container/inline_vector.h"
#include "core/diffusion_grid.h"
#include "core/environment/environment.h"
#include "core/environment/environment.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

namespace bdm {

/// A class that sets up diffusion grids of the substances in this simulation
class DiffusionOp : public OperationImpl {
 public:
  DiffusionOp() {}
  virtual ~DiffusionOp() {}

  bool IsRowWise() override { return false; }

  DiffusionOp* Clone() override { return new DiffusionOp(*this); }

  void operator()() override {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    auto* env = sim->GetEnvironment();
    auto* param = sim->GetParam();

    rm->ApplyOnAllDiffusionGrids([&](DiffusionGrid* dg) {
      // Update the diffusion grid dimension if the environment dimensions
      // have changed. If the space is bound, we do not need to update the
      // dimensions, because these should not be changing anyway
      if (env->HasGrown() && !param->bound_space_) {
        Log::Info("DiffusionOp",
                  "Your simulation objects are getting near the edge of the "
                  "simulation space. Be aware of boundary conditions that may "
                  "come into play!");
        dg->Update(env->GetDimensionThresholds());
      }
      if (param->diffusion_type_ == "RK") {
        if (param->leaking_edges_) {
          dg->RKLeaking();
        } else {
          dg->RK();
        }

      } else {
        if (param->leaking_edges_) {
          dg->DiffuseEulerLeakingEdge();
        } else {
          dg->DiffuseEuler();
        }
      }
      if (param->calculate_gradients_) {
        dg->CalculateGradient();
      }
    });
  }

 private:
  static bool registered_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_DIFFUSION_OP_H_
