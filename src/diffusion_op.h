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

#include "bdm.h"
#include "cell.h"
#include "diffusion_grid.h"
#include "grid.h"
#include "inline_vector.h"
#include "resource_manager.h"

namespace bdm {

/// A class that sets up diffusion grids of the substances in this simulation
class DiffusionOp {
 public:
  DiffusionOp() {}
  virtual ~DiffusionOp() {}

  template <typename TContainer, typename TBdmSim = BdmSim<>>
  void operator()(TContainer* cells, uint16_t type_idx) {
    auto* sim = TBdmSim::GetBdm();
    auto* grid = sim->GetGrid();
    auto& diffusion_grids = sim->GetRm()->GetDiffusionGrids();
    for (auto dg : diffusion_grids) {
      // Update the diffusion grid dimension if the neighbor grid dimensions
      // have changed. If the space is bound, we do not need to update the
      // dimensions, because these should not be changing anyway
      if (grid->HasGrown() && !Param::bound_space_) {
        Log::Info("DiffusionOp",
                  "Your simulation objects are getting near the edge of the "
                  "simulation space. Be aware of boundary conditions that may "
                  "come into play!");
        dg->Update(grid->GetDimensionThresholds());
      }

      dg->DiffuseEuler();

      if (Param::calculate_gradients_) {
        dg->CalculateGradient();
      }
    }
  }
};

}  // namespace bdm

#endif  // DIFFUSION_OP_H_
