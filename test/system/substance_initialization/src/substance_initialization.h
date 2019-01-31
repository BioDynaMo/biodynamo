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

#ifndef INTEGRATION_SUBSTANCE_INITIALIZATION_H_
#define INTEGRATION_SUBSTANCE_INITIALIZATION_H_

#include <vector>

#include "biodynamo.h"
#include "core/substance_initializers.h"

namespace bdm {

// -----------------------------------------------------------------------------
// In this integration test we should how to make use of the 'substance
// initializers', in order to initialize the concentration of a particular
// substance. We create a gaussian distribution along each axis.
// -----------------------------------------------------------------------------

// Create list of substances
enum Substances { kSubstance };

inline int Simulate(int argc, const char** argv) {
  auto set_param = [](auto* param) {
    // Create an artificial bounds for the simulation space
    param->bound_space_ = true;
    param->min_bound_ = -100;
    param->max_bound_ = 100;
  };

  Simulation simulation(argc, argv, set_param);
  auto* param = simulation.GetParam();

  // Define initial model
  // Create one cell at a random position
  auto construct = [](const std::array<double, 3>& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_, 1,
                                      construct);

  // Define the substances in our simulation
  // Order: substance id, substance_name, diffusion_coefficient, decay_constant,
  // resolution
  ModelInitializer::DefineSubstance(kSubstance, "Substance", 0.5, 0, 20);

  // Order: substance id, substance name, initialization model, along which axis
  // (0 = x, 1 = y, 2 = z). See the documentation of `GaussianBand` for
  // information about its arguments
  ModelInitializer::InitializeSubstance(kSubstance, "Substance",
                                        GaussianBand(0, 5, Axis::kXAxis));
  ModelInitializer::InitializeSubstance(kSubstance, "Substance",
                                        GaussianBand(0, 5, Axis::kYAxis));
  ModelInitializer::InitializeSubstance(kSubstance, "Substance",
                                        GaussianBand(0, 5, Axis::kZAxis));

  // Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(20);

  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm

#endif  // INTEGRATION_SUBSTANCE_INITIALIZATION_H_
