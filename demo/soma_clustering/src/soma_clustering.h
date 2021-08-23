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
// This model examplifies the use of extracellur diffusion and shows
// how to extend the default "Cell". In step 0 one can see how an extra
// data member is added and can be accessed throughout the simulation with
// its Get and Set methods. N cells are randomly positioned in space, of which
// half are of type 1 and half of type -1. Each type secretes a different
// substance. Cells move towards the gradient of their own substance, which
// results in clusters being formed of cells of the same type.
//

#ifndef DEMO_SOMA_CLUSTERING_H_
#define DEMO_SOMA_CLUSTERING_H_

#include <vector>

#include "biodynamo.h"
#include "my_cell.h"
#include "validation_criterion.h"

namespace bdm {
namespace soma_clustering {

enum Substances { kSubstance0, kSubstance1 };

inline int Simulate(int argc, const char** argv) {
  auto set_param = [](Param* param) {
    // Create an artificial bound for the simulation space
    param->bound_space = Param::BoundSpaceMode::kClosed;
    param->min_bound = 0;
    param->max_bound = 250;
    param->unschedule_default_operations = {"mechanical forces"};
  };

  Simulation simulation(argc, argv, set_param);

  // Define initial model
  auto* param = simulation.GetParam();
  int num_cells = 20000;

#pragma omp parallel
  simulation.GetRandom()->SetSeed(4357);

  // Define the substances that cells may secrete
  // Order: substance_name, diffusion_coefficient, decay_constant, resolution
  ModelInitializer::DefineSubstance(kSubstance0, "Substance_0", 0.5, 0.1, 20);
  ModelInitializer::DefineSubstance(kSubstance1, "Substance_1", 0.5, 0.1, 20);

  int cell_type = 1;
  std::string substance_name = "Substance_0";

  auto construct = [&cell_type, &substance_name](const Double3& position) {
    auto* cell = new MyCell(position, cell_type);
    cell->SetDiameter(10);
    cell->AddBehavior(new Secretion(substance_name));
    cell->AddBehavior(new Chemotaxis(substance_name, 5));
    return cell;
  };

  // Construct num_cells/2 cells of type 0
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound,
                                       num_cells / 2, construct);
  // Construct num_cells/2 cells of type 1
  cell_type = -1;
  substance_name = "Substance_1";
  ModelInitializer::CreateAgentsRandom(param->min_bound, param->max_bound,
                                       num_cells / 2, construct);

  // Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(1000);

  // Check if criterion is met
  double spatial_range = 5;
  auto crit = GetCriterion(spatial_range, num_cells / 8);
  if (crit) {
    std::cout << "Simulation completed successfully!\n";
  }
  return !crit;
}

}  // namespace soma_clustering
}  // namespace bdm

#endif  // DEMO_SOMA_CLUSTERING_H_
