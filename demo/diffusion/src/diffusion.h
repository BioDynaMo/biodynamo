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
  auto* rm = simulation.GetResourceManager();
  auto* dgrid = rm->GetDiffusionGrid(kKalium);

  auto construct = [&](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(30);
    cell->SetMass(1.0);
    Double3 secretion_position = {{50, 50, 50}};
    if (position == secretion_position) {
      cell->AddBehavior(new Secretion(dgrid, 4));
    } else {
      cell->AddBehavior(new Chemotaxis(dgrid, 0.5));
    }
    return cell;
  };
  std::vector<Double3> positions;
  positions.push_back({0, 0, 0});
  positions.push_back({100, 0, 0});
  positions.push_back({0, 100, 0});
  positions.push_back({0, 0, 100});
  positions.push_back({0, 100, 100});
  positions.push_back({100, 0, 100});
  positions.push_back({100, 100, 0});
  positions.push_back({100, 100, 100});
  // The cell responsible for secretion
  positions.push_back({50, 50, 50});
  ModelInitializer::CreateAgents(positions, construct);

  // Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(300);
  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm

#endif  // DEMO_DIFFUSION_H_
