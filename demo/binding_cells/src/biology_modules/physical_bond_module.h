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
#ifndef PHYSICAL_BOND_MODULE_H_
#define PHYSICAL_BOND_MODULE_H_

#include "biodynamo.h"
#include "simulation_objects/monocyte.h"

namespace bdm {

/// If a cell A establishes a PhysicalBond with another cell B, cell A
/// attaches itself to B. Any displacement of cell B will result in cell A
/// moving next to cell B. This effectively disables cell A from being able to
/// move by itself.
struct PhysicalBond : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(PhysicalBond, BaseBiologyModule, 1);

 public:
  PhysicalBond() : BaseBiologyModule(gAllEventIds) {}

  // NB: works only for spherical shaped cells
  // Simple implementation of connecting `cell_a` to `cell_b`. We displace
  // `cell_a` with a distance such that the edges of the spheres stay connected
  void Connect(TCell* cell_a, AgentPointer<Monocyte> cell_b) {
    Double3 distance = cell_b->GetPosition() - cell_a->GetPosition();
    Double3 distance_copy = distance;
    auto radius_a = cell_a->GetDiameter() / 2;
    auto radius_b = cell_b->GetDiameter() / 2;
    Double3 displacement =
        distance - (distance_copy.Normalize() * (radius_a + radius_b));
    cell_a->UpdatePosition(displacement);
    // if (Simulation::GetActive()->GetScheduler()->GetSimulatedSteps() == 1) {
    //   Log::Error("PhysicalBond", "Displacement: ", displacement);
    // }
  }

  void Run(Agent* agent) override {
    if (auto* this_cell = static_cast<TCell*>(agent)) {
      if (this_cell->IsConnected()) {
        auto other_cell = this_cell->GetConnectedCell();
        Connect(this_cell, other_cell);
      }
    }
  }
};

}  // namespace bdm

#endif  // PHYSICAL_BOND_MODULE_H_
