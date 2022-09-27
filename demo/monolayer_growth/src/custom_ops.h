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

#ifndef CUSTOM_OPS_H_
#define CUSTOM_OPS_H_

#include "biodynamo.h"
#include "core/environment/uniform_grid_environment.h"

namespace bdm {

// Brings cells back to the xy plane (i.e. z = 0)
struct MoveCellsPlane : public AgentOperationImpl {
  BDM_OP_HEADER(MoveCellsPlane);

  void operator()(Agent* agent) override {
    Double3 cell_position = agent->GetPosition();
    if (cell_position[2] != 0.0) {
      cell_position[2] = 0.0;
      agent->SetPosition(cell_position);
    }
  }
};

// Counts the number of cells
struct CountCells : public StandaloneOperationImpl {
  BDM_OP_HEADER(CountCells);

  void operator()() override {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    (*total_cells_).push_back(rm->GetNumAgents());
  }

  std::vector<size_t>* total_cells_;
};

}  // namespace bdm

#endif
