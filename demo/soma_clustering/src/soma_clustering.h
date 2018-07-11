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

#ifndef SOMA_CLUSTERING_H_
#define SOMA_CLUSTERING_H_

#include <vector>

#include "biodynamo.h"
#include "my_cell.h"
#include "soma_clustering_biology_modules.h"
#include "validation_criterion.h"

namespace bdm {

// ----------------------------------------------------------------------------
// This model examplifies the use of extracellur diffusion and shows
// how to extend the default "Cell". In step 0 one can see how an extra
// data member is added and can be accessed throughout the simulation with
// its Get and Set methods. N cells are randomly positioned in space, of which
// half are of type 1 and half of type -1.
//
// Each type secretes a different substance. Cells move towards the gradient of
// their own substance, which results in clusters being formed of cells of the
// same type.
// -----------------------------------------------------------------------------

// Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<Chemotaxis, SubstanceSecretion>;
  using AtomicTypes = VariadicTypedef<MyCell>;
};

inline int Simulate(int argc, const char** argv) {
  Simulation<> simulation(argc, argv);
  auto* param = simulation.GetParam();

  // Define initial model
  // Create an artificial bounds for the simulation space
  param->bound_space_ = true;
  param->min_bound_ = 0;
  param->max_bound_ = 250;
  param->run_mechanical_interactions_ = false;
  int num_cells = 20000;

#pragma omp parallel
  simulation.GetRandom()->SetSeed(4357);

  // Construct num_cells/2 cells of type 1
  auto construct_0 = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(10);
    cell.SetCellType(1);
    cell.AddBiologyModule(SubstanceSecretion());
    cell.AddBiologyModule(Chemotaxis());
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_,
                                      num_cells / 2, construct_0);

  // Construct num_cells/2 cells of type -1
  auto construct_1 = [](const std::array<double, 3>& position) {
    MyCell cell(position);
    cell.SetDiameter(10);
    cell.SetCellType(-1);
    cell.AddBiologyModule(SubstanceSecretion());
    cell.AddBiologyModule(Chemotaxis());
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_,
                                      num_cells / 2, construct_1);

  // Define the substances that cells may secrete
  // Order: substance_name, diffusion_coefficient, decay_constant, resolution
  ModelInitializer::DefineSubstance(kSubstance0, "Substance_0", 0.5, 0.1, 20);
  ModelInitializer::DefineSubstance(kSubstance1, "Substance_1", 0.5, 0.1, 20);

  // Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(1000);

  double spatial_range = 5;
  auto crit = GetCriterion(spatial_range, num_cells / 8);
  if (crit) {
    std::cout << "Simulation completed successfully!\n";
  }
  return !crit;
}

}  // namespace bdm

#endif  // SOMA_CLUSTERING_H_
