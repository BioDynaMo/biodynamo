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

namespace bdm {

enum Substances { kSubstance0, kSubstance1 };

// Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis : public BaseBiologyModule {
  Chemotaxis() : BaseBiologyModule(gAllBmEvents) {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    if (!init_) {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      dg_0_ = rm->GetDiffusionGrid(kSubstance0);
      dg_1_ = rm->GetDiffusionGrid(kSubstance1);
      init_ = true;
    }

    auto& position = cell->GetPosition();
    std::array<double, 3> diff_gradient;

    if (cell->GetCellType() == 1) {
      dg_1_->GetGradient(position, &gradient_1_);
      diff_gradient = Math::ScalarMult(5, gradient_1_);
    } else {
      dg_0_->GetGradient(position, &gradient_0_);
      diff_gradient = Math::ScalarMult(5, gradient_0_);
    }

    cell->UpdatePosition(diff_gradient);
  }

 private:
  bool init_ = false;
  DiffusionGrid* dg_0_ = nullptr;
  DiffusionGrid* dg_1_ = nullptr;
  std::array<double, 3> gradient_0_{};
  std::array<double, 3> gradient_1_{};
  ClassDefNV(Chemotaxis, 1);
};

// Define secretion behavior:
struct SubstanceSecretion : public BaseBiologyModule {
  SubstanceSecretion() : BaseBiologyModule(gAllBmEvents) {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    if (!init_) {
      auto* rm = TSimulation::GetActive()->GetResourceManager();
      dg_0_ = rm->GetDiffusionGrid(kSubstance0);
      dg_1_ = rm->GetDiffusionGrid(kSubstance1);
      init_ = true;
    }
    auto& secretion_position = cell->GetPosition();
    if (cell->GetCellType() == 1) {
      dg_1_->IncreaseConcentrationBy(secretion_position, 1);
    } else {
      dg_0_->IncreaseConcentrationBy(secretion_position, 1);
    }
  }

 private:
  bool init_ = false;
  DiffusionGrid* dg_0_ = nullptr;
  DiffusionGrid* dg_1_ = nullptr;
  ClassDefNV(SubstanceSecretion, 1);
};

}  // namespace bdm

#endif  // SOMA_CLUSTERING_BIOLOGY_MODULES_H_
