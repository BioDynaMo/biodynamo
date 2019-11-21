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
#ifndef GRAVITY_MODULE_H_
#define GRAVITY_MODULE_H_

#include "biodynamo.h"
#include "simulation_objects/my_cell.h"

namespace bdm {

/// If a cell A establishes a Gravity with another cell B, cell A
/// attaches itself to B. Any displacement of cell B will result in cell A
/// moving next to cell B. This effectively disables cell A from being able to
/// move by itself.
struct Gravity : public BaseBiologyModule {
  BDM_STATELESS_BM_HEADER(Gravity, BaseBiologyModule, 1);

 public:
  Gravity() : BaseBiologyModule(gAllEventIds) {}

  static constexpr double kG = -1;

  void Run(SimObject* so) override {
    if (auto* cell = dynamic_cast<MyCell*>(so)) {
      if (!cell->IsConnected()) {
        cell->UpdatePosition({0, 0, kG});
      }
    }
  }
};

}  // namespace bdm

#endif  // GRAVITY_MODULE_H_
