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

#ifndef SOMA_CLUSTERING_BIOLOGY_MODULES_H_
#define SOMA_CLUSTERING_BIOLOGY_MODULES_H_

#include "biodynamo.h"
#include "my_cell.h"

namespace bdm {

enum Substances { kSubstance0, kSubstance1 };

// Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(Chemotaxis, BaseBiologyModule, 1);

  Chemotaxis() : BaseBiologyModule(gAllEventIds) {}

  void Run(SimObject* so) override {
    if (auto* cell = dynamic_cast<MyCell*>(so)) {
      auto* rm = Simulation::GetActive()->GetResourceManager();

      DiffusionGrid* dg = nullptr;
      if (cell->GetCellType() == 1) {
        dg = rm->GetDiffusionGrid(kSubstance0);
      } else {
        dg = rm->GetDiffusionGrid(kSubstance1);
      }

      auto& position = cell->GetPosition();
      Double3 gradient;
      Double3 diff_gradient;

      dg->GetGradient(position, &gradient);
      diff_gradient = gradient * 5;
      cell->UpdatePosition(diff_gradient);
    }
  }
};

// Define secretion behavior:
struct SubstanceSecretion : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(SubstanceSecretion, BaseBiologyModule, 1);

  SubstanceSecretion() : BaseBiologyModule(gAllEventIds) {}

  void Run(SimObject* so) override {
    if (auto* cell = dynamic_cast<MyCell*>(so)) {
      auto* rm = Simulation::GetActive()->GetResourceManager();

      DiffusionGrid* dg = nullptr;
      if (cell->GetCellType() == 1) {
        dg = rm->GetDiffusionGrid(kSubstance0);
      } else {
        dg = rm->GetDiffusionGrid(kSubstance1);
      }

      auto& secretion_position = cell->GetPosition();
      dg->IncreaseConcentrationBy(secretion_position, 1);
    }
  }
};

}  // namespace bdm

#endif  // SOMA_CLUSTERING_BIOLOGY_MODULES_H_
