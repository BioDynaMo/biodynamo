// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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
//
// \visualize
//
// This model creates 8 cells at each corner of a cube, and one in the middle.
// The cell in the middle secretes a substance. The cells are modeled to move
// according to the extracellular gradient; in this case to the middle.
//

#ifndef DEMO_DIFFUSION_H_
#define DEMO_DIFFUSION_H_

#include "biodynamo.h"

namespace bdm {

// List the extracellular substances
enum Substances { kKalium };

inline int Simulate(int argc, const char** argv) {
  // Initialize BioDynaMo
  Simulation simulation(argc, argv);

  // Define the substances that cells may secrete
  ModelInitializer::DefineSubstance(kKalium, "Kalium", 0.4, 0, 25);

  // Create 8 cells in a 2x2x2 grid setup
  auto construct = [&](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(30);
    cell->SetMass(1.0);
    cell->AddBehavior(new Chemotaxis("Kalium", 0.5));
    return cell;
  };
  ModelInitializer::Grid3D(2, 100, construct);

  // The cell responsible for secretion
  auto* secreting_cell = new Cell({50, 50, 50});
  secreting_cell->AddBehavior(new Secretion("Kalium", 4));
  simulation.GetResourceManager()->AddAgent(secreting_cell);

  // Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(300);
  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm

#endif  // DEMO_DIFFUSION_H_
