// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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
// \title Cell Division
// \visualize
//
// This model creates a grid of 4x4x4 cells. Each cell grows until a specific
// volume, after which it proliferates (i.e. divides).
//

#ifndef DEMO_CELL_DIVISION_H_
#define DEMO_CELL_DIVISION_H_

#include "biodynamo.h"

namespace bdm {
namespace cell_division {

inline int Simulate(int argc, const char** argv) {
  // Create a new simulation
  Simulation simulation(argc, argv);

  // Let's define the number of cells we wish to create along each dimension,
  // the spacing between the cells, and each cell's diameter.
  size_t cells_per_dim = 4;
  size_t spacing = 20;

  // To define how are cells will look like we will create a construct in the
  // form of a C++ lambda as follows.
  auto construct = [&](const Real3& position) {
    size_t diameter_min = 5;
    size_t diameter_max = 10;
    real_t growth_ratio = 50;

    Cell* cell = new Cell(position);
    cell->SetDiameter(diameter_min);
    // Add the "grow and divide" behavior to each cell
    cell->AddBehavior(new GrowthDivision(diameter_max, growth_ratio));
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, spacing, construct);

  // Run simulation for a few time-steps
  simulation.GetScheduler()->Simulate(111);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace cell_division
}  // namespace bdm

#endif  // DEMO_CELL_DIVISION_H_
