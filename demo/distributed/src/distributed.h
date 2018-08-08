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

#ifndef DEMO_DISTRIBUTED_DISTRIBUTED_H_
#define DEMO_DISTRIBUTED_DISTRIBUTED_H_

// 1. Define the backend.
#include "backend.h"

#include "bdm_ray.h"

extern bool g_under_ray;

namespace bdm {

inline int Simulate(int argc, const char** argv) {
  // 2. Create new simulation
  std::unique_ptr<Simulation<>> simulation;
  if (!g_under_ray) {
    simulation.reset(new Simulation<>(argc, argv));
  } else {
    simulation.reset(new RaySimulation(argc, argv));
  }

  // 3. Define initial model - in this example: 3D grid of cells
  size_t cells_per_dim = 16;
  if (argc > 1) {
    if (sscanf(argv[1], "%lu", &cells_per_dim) != 1) {
      std::cerr << "argv[1] is not a decimal number and is ignored.\n";
    }
  }
  std::cout << "cells_per_dim " << cells_per_dim << '\n';
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(10);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(GrowDivide());
    return cell;
  };
  ModelInitializer::Grid3D(cells_per_dim, 20, construct);

  // 4. Run simulation for some steps.
  size_t step_count = 50;
  if (argc > 2) {
    if (sscanf(argv[2], "%lu", &step_count) != 1) {
      std::cerr << "argv[2] is not a decimal number and is ignored.\n";
    }
  }
  std::cout << "step_count " << step_count << '\n';
  {
    Timing timer("Simulate time");
    simulation->GetScheduler()->Simulate(step_count);
  }
  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm
#endif  // DEMO_DISTRIBUTED_DISTRIBUTED_H_
