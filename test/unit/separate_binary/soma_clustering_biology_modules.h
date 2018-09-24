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

class MyCell;

// Define displacement behavior:
// Cells move along the diffusion gradient (from low concentration to high)
struct Chemotaxis : public BaseBiologyModule {
  Chemotaxis() : BaseBiologyModule(gAllEventIds) {}

  /// Empty default event constructor, because Chemotaxis does not have state.
  Chemotaxis(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) {}

  BaseBiologyModule* New(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) const override {
    return new Chemotaxis(event, dynamic_cast<Chemotaxis*>(other), new_oid);
  }

  /// Empty default event handler, because Chemotaxis does not have state.
  void EventHandler(const Event&, BaseBiologyModule* other) override {}

  void Run(SimulationObject* so) override {
    if(auto* cell = dynamic_cast<MyCell*>(so)) {
      auto* rm = Simulation::GetActive()->GetResourceManager();

      DiffusionGrid* dg = nullptr;
      if (cell->GetCellType() == 1) {
        dg = rm->GetDiffusionGrid(kSubstance0);
      } else {
        dg = rm->GetDiffusionGrid(kSubstance1);
      }

      auto& position = cell->GetPosition();
      std::array<double, 3> gradient;
      std::array<double, 3> diff_gradient;

      dg->GetGradient(position, &gradient);
      diff_gradient = Math::ScalarMult(5, gradient);
      cell->UpdatePosition(diff_gradient);
    } else {
      Log::Fatal("Chemotaxis", "Run only accepts MyCell");
    }
  }

 private:
  // ClassDefNV(Chemotaxis, 1);
};

// Define secretion behavior:
struct SubstanceSecretion : public BaseBiologyModule {
  SubstanceSecretion() : BaseBiologyModule(gAllEventIds) {}

  /// Empty default event constructor, because SubstanceSecretion does not have
  /// state.
  SubstanceSecretion(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) {}

  BaseBiologyModule* New(const Event& event, BaseBiologyModule* other, uint64_t new_oid = 0) const override {
    return new SubstanceSecretion(event, dynamic_cast<SubstanceSecretion*>(other), new_oid);
  }

  /// Empty default event handler, because SubstanceSecretion does not have
  /// state.
  void EventHandler(const Event&, BaseBiologyModule* other) override {}

  void Run(SimulationObject* so) override {
    if(auto* cell = dynamic_cast<MyCell*>(so)) {
      auto* rm = Simulation::GetActive()->GetResourceManager();

      DiffusionGrid* dg = nullptr;
      if (cell->GetCellType() == 1) {
        dg = rm->GetDiffusionGrid(kSubstance0);
      } else {
        dg = rm->GetDiffusionGrid(kSubstance1);
      }

      auto& secretion_position = cell->GetPosition();
      dg->IncreaseConcentrationBy(secretion_position, 1);
    } else {
      Log::Fatal("SubstanceSecretion", "Run only accepts MyCell");
    }
  }

 private:
  ClassDefNV(SubstanceSecretion, 1);
};

}  // namespace bdm

#endif  // SOMA_CLUSTERING_BIOLOGY_MODULES_H_
