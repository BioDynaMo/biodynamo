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

#ifndef DIFFUSION_OP_H_
#define DIFFUSION_OP_H_

#include <string>
#include <utility>
#include <vector>

#include "diffusion_grid.h"
#include "grid.h"
#include "inline_vector.h"
#include "resource_manager.h"
#include "simulation.h"

namespace bdm {

/// A class that sets up diffusion grids of the substances in this simulation
class DiffusionOp {
 public:
  DiffusionOp() {}
  virtual ~DiffusionOp() {}

  template <typename TContainer, typename TSimulation = Simulation<>>
  void operator()(TContainer* cells, uint16_t type_idx) {
    auto* sim = TSimulation::GetActive();
    auto* grid = sim->GetGrid();
    auto* param = sim->GetParam();
    auto& diffusion_grids = sim->GetResourceManager()->GetDiffusionGrids();
    for (auto dg : diffusion_grids) {
      // Update the diffusion grid dimension if the neighbor grid dimensions
      // have changed. If the space is bound, we do not need to update the
      // dimensions, because these should not be changing anyway
      if (grid->HasGrown() && !param->bound_space_) {
        Log::Info("DiffusionOp",
                  "Your simulation objects are getting near the edge of the "
                  "simulation space. Be aware of boundary conditions that may "
                  "come into play!");
        dg->Update(grid->GetDimensionThresholds());
      }

      dg->DiffuseEuler();

      if (param->calculate_gradients_) {
        dg->CalculateGradient();
      }
    }
  }
};

}  // namespace bdm

#endif  // DIFFUSION_OP_H_
