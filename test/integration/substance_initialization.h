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
#include "substance_initializers.h"

namespace bdm {

// -----------------------------------------------------------------------------
// In this integration test we should how to make use of the 'substance
// initializers', in order to initialize the concentration of a particular
// substance. We create a gaussian distribution along each axis.
// -----------------------------------------------------------------------------

// 1. Create list of substances
enum Substances { kSubstance };

// 2. Use default compile-time parameters to let the compiler know we are not
// using any new biology modules or cell types
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {};

inline int Simulate(int argc, const char** argv) {
  Simulation<> simulation(argc, argv);
  auto* param = simulation.GetParam();

  // 3. Define initial model
  // Create an artificial bounds for the simulation space
  param->bound_space_ = true;
  param->min_bound_ = -100;
  param->max_bound_ = 100;

  // Create one cell at a random position
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(10);
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_, 1,
                                      construct);

  // 3. Define the substances in our simulation
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

  // 4. Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(20);

  return 0;
}

}  // namespace bdm

#endif  // INTEGRATION_SUBSTANCE_INITIALIZATION_H_
