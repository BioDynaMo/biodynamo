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
  Chemotaxis() : BaseBiologyModule(gAllEventIds) {}

  /// Empty default event constructor, because Chemotaxis does not have state.
  template <typename TEvent, typename TBm>
  Chemotaxis(const TEvent& event, TBm* other, uint64_t new_oid = 0) {}

  /// event handler not needed, because Chemotaxis does not have state.

  template <typename T, typename TSimulation = Simulation>
  void Run(T* cell) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* kDg = rm->GetDiffusionGrid(kKalium);
    kDg->SetConcentrationThreshold(1e15);

    auto& position = cell->GetPosition();
    std::array<double, 3> gradient;
    kDg->GetGradient(position, &gradient);
    gradient[0] *= 0.5;
    gradient[1] *= 0.5;
    gradient[2] *= 0.5;

    cell->UpdatePosition(gradient);
  }

 private:
  BDM_CLASS_DEF_NV(Chemotaxis, 1);
};

// Define secretion behavior:
// One cell is assigned to secrete Kalium artificially at one location
struct KaliumSecretion : public BaseBiologyModule {
  KaliumSecretion() : BaseBiologyModule() {}

  /// Empty default event constructor, because KaliumSecretion does not have
  /// state.
  template <typename TEvent, typename TBm>
  KaliumSecretion(const TEvent& event, TBm* other, uint64_t new_oid = 0) {}

  /// event handler not needed, because Chemotaxis does not have state.

  template <typename T, typename TSimulation = Simulation>
  void Run(T* cell) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    static auto* kDg = rm->GetDiffusionGrid(kKalium);
    double amount = 4;
    kDg->IncreaseConcentrationBy(cell->GetPosition(), amount);
  }

 private:
  BDM_CLASS_DEF_NV(KaliumSecretion, 1);
};

}  // namespace bdm

#endif  // DIFFUSION_BIOLOGY_MODULES_H_
