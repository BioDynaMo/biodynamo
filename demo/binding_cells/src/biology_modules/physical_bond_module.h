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
#include "simulation_objects/my_cell.h"

namespace bdm {

/// If a cell A establishes a PhysicalBond with another cell B, cell A
/// attaches itself to B. Any displacement of cell B will result in cell A
/// moving next to cell B. This effectively disables cell A from being able to
/// move by itself.
struct PhysicalBond : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(PhysicalBond, BaseBiologyModule, 1);

 public:
  PhysicalBond() : BaseBiologyModule(gAllEventIds) {}

  void Connect(MyCell* cell_a, SoPointer<MyCell> cell_b) {
    Double3 distance = cell_b->GetPosition() - cell_a->GetPosition();
    Double3 distance_copy = distance;
    auto radius_a = cell_a->GetDiameter() / 2;
    auto radius_b = cell_b->GetDiameter() / 2;
    Double3 displacement =
        distance - (distance_copy.Normalize() * (radius_a + radius_b));
    cell_a->UpdatePosition(displacement);
  }

  void Run(SimObject* so) override {
    if (auto* this_cell = dynamic_cast<MyCell*>(so)) {
      if (this_cell->IsConnected()) {
        auto other_cell = this_cell->GetConnectedCell();
        Connect(this_cell, other_cell);
      }
    }
  }
};

}  // namespace bdm

#endif  // PHYSICAL_BOND_MODULE_H_
