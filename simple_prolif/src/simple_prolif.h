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
#ifndef SIMPLE_PROLIF_H_
#define SIMPLE_PROLIF_H_

#include "biodynamo.h"
#include "my_funcs.h"
#include "my_cell.h"
#include "my_dynamics.h"

#include "core/substance_initializers.h"
//#include <iostream>
//using namespace std;

namespace bdm {

// List the extracellular substances
enum Substances { GuidanceCue };

inline int Simulate(int argc, const char** argv) {

  auto set_param = [](Param* param) {
    // Create an artificial bound for the simulation space
    param->bound_space = true;
    param->min_bound = 0;
    param->max_bound = 250;
    param->unschedule_default_operations = {"mechanical forces"};
  };

  Simulation simulation(argc, argv);

  auto* param = simulation.GetParam();

  // Define the substances that cells may secrete
  ModelInitializer::DefineSubstance(GuidanceCue, "GuidanceCue", 1., 0, 10);
  //ModelInitializer::InitializeSubstance(GuidanceCue, PoissonBand(1, Axis::kZAxis));
  Double3 gradPos = {0.,0.,3.};
  ModelInitializer::InitializeSubstance(GuidanceCue, PoissonBandAtPos(100, Axis::kZAxis, gradPos));

  // Let's define the number of cells we wish to create along each dimension,
  // the spacing between the cells, and each cell's diameter.
  size_t cells_per_dim = 4;
  size_t spacing = 20;
  size_t diameter = 10;

  // To define how are cells will look like we will create a construct in the
  // form of a C++ lambda as follows.
  auto construct = [&](const Double3& position) {
    MyCell* cell = new MyCell(position, 0, 0, "GuidanceCue");
    //MyCell* cell = new MyCell(position, 1, 0);
    cell->SetDiameter(diameter);
    // Add the "grow and divide" behavior to each cell
    //cell->AddBehavior(new GrowthDivision(25, 3000));
    cell->AddBehavior(new MyGrowth());

    return cell;
  };
  //ModelInitializer::Grid2D(cells_per_dim, spacing, construct);
  Grid2D(cells_per_dim, spacing, construct);

  // Define initial model - in this example: single cell at origin
  //auto* rm = simulation.GetResourceManager();
  //auto* cell = new Cell(30);
  //rm->AddAgent(cell);

  // Run simulation for one timestep
  for (int i = 0; i<1000; i++) {
    simulation.GetScheduler()->Simulate(1);
    std::cout << "hello " << i << std::endl;
}

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // SIMPLE_PROLIF_H_
