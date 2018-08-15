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
#ifndef CELLS_MOVING_H_
#define CELLS_MOVING_H_

#include "biodynamo.h"
#include "substance_initializers.h"

namespace bdm {

// -----------------------------------------------------------------------------
// This model creates random number of cells from (15 to 25) at random positions
// of simulation space.
// Cells are sensing the extracellular gradient and moving according to it. The
// gradient belongs to the substance that is concentrated on the plane with 
// equation By + Cz = 0 (passes through x-axis).
// Due to the fact that diffusion is occuring during the simulation (otherwise 
// there would be no gradient) the substance spreads in the space, so its 
// concentartion will be slightly higher near the origin (0;0;0), than at 
// peripheral regions. That's why cells continue moving even after reachig the 
// scope of substance.
// -----------------------------------------------------------------------------

// 0. List the extracellular substances
enum Substances { kSubstance };

// 1. Define displacement behavior.
// Cells moving from low concentrations of substance to high 
// according to the substance gradient.
struct Chemotaxis : public BaseBiologyModule {
  Chemotaxis() : BaseBiologyModule(gAllBmEvents) {}

  template <typename T, typename TSimulation = Simulation<>>
  void Run(T* cell) {
    auto* sim = TSimulation::GetActive();
    auto* rm = sim->GetResourceManager();
    // Get Diffusion parameters
    static auto* kDg = rm->GetDiffusionGrid(kSubstance);
    kDg->SetConcentrationThreshold(1e15);

    auto& position = cell->GetPosition();
    std::array<double, 3> gradient;
    // Compute gradient of the substance at particular position
    kDg->GetGradient(position, &gradient);
    gradient[0] *= 0.5;
    gradient[1] *= 0.5;
    gradient[2] *= 0.5;

    cell->UpdatePosition(gradient);
  }

  ClassDefNV(Chemotaxis, 1);
};

// Define compile time parameter
template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<Chemotaxis>; // add Chemotaxis
};

inline int Simulate(int argc, const char** argv) {
  Simulation<> simulation(argc, argv);
  auto* param = simulation.GetParam();
  auto* random = simulation.GetActive()->GetRandom();

  // 2. Define initial model
  // Create an artificial bounds for the simulation space
  param->bound_space_ = true;
  param->min_bound_ = -100;
  param->max_bound_ = 100;

  // random number of cells used in simulation
  size_t nb_of_cells = random->Uniform(15, 25);
  // create nb_of_cells cells at random positions
  auto construct = [](const std::array<double, 3>& position) {
    Cell cell(position);
    cell.SetDiameter(10);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(Chemotaxis());
    return cell;
  };
  ModelInitializer::CreateCellsRandom(param->min_bound_, param->max_bound_, 
						nb_of_cells, construct);

  // 3. Define the substance towards which cells will be moving
  ModelInitializer::DefineSubstance(kSubstance, "Substance", 0.1, 0, 20);

  ModelInitializer::InitializeSubstance(kSubstance, "Substance", 
					GaussianBand(0, 5, Axis::kXAxis));

  // 4. Run simulation for N timesteps
  simulation.GetScheduler()->Simulate(1000);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // CELLS_MOVING_H_
