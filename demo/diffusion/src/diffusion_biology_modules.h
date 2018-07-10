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
  Chemotaxis() : BaseBiologyModule(gAllBmEvents) {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* dg = rm->GetDiffusionGrid(kKalium);
    dg->SetConcentrationThreshold(1e15);

    auto& position = cell->GetPosition();
    std::array<double, 3> gradient;
    dg->GetGradient(position, &gradient);
    gradient[0] *= 0.5;
    gradient[1] *= 0.5;
    gradient[2] *= 0.5;

    cell->UpdatePosition(gradient);
  }

  ClassDefNV(Chemotaxis, 1);
};

// Define secretion behavior:
// One cell is assigned to secrete Kalium artificially at one location
struct KaliumSecretion : public BaseBiologyModule {
  KaliumSecretion() : BaseBiologyModule() {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* dg = rm->GetDiffusionGrid(kKalium);
    double amount = 4;
    dg->IncreaseConcentrationBy(cell->GetPosition(), amount);
  }

  ClassDefNV(KaliumSecretion, 1);
};

}  // namespace bdm

#endif  // DIFFUSION_BIOLOGY_MODULES_H_
