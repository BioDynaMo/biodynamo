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

#ifndef DIFFUSION_BIOLOGY_MODULES_H_
#define DIFFUSION_BIOLOGY_MODULES_H_

#include "biodynamo.h"

namespace bdm {

// List the extracellular substances
enum Substances { kKalium };

// Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(Chemotaxis, BaseBiologyModule, 1);

  Chemotaxis() : BaseBiologyModule(gAllEventIds) {}

  void Run(SimObject* so) override {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* kDg = rm->GetDiffusionGrid(kKalium);

    if (auto* cell = dynamic_cast<Cell*>(so)) {
      const auto& position = so->GetPosition();
      Double3 gradient;
      kDg->GetGradient(position, &gradient);
      gradient *= 0.5;
      cell->UpdatePosition(gradient);
    }
  }
};

// Define secretion behavior:
// One cell is assigned to secrete Kalium artificially at one location
struct KaliumSecretion : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(KaliumSecretion, BaseBiologyModule, 1);

  KaliumSecretion() : BaseBiologyModule() {}

  void Run(SimObject* so) override {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* kDg = rm->GetDiffusionGrid(kKalium);
    double amount = 4;
    kDg->IncreaseConcentrationBy(so->GetPosition(), amount);
  }
};

}  // namespace bdm

#endif  // DIFFUSION_BIOLOGY_MODULES_H_
