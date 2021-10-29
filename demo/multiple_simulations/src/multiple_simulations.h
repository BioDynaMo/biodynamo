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

#ifndef DEMO_MULTIPLE_SIMULATIONS_H_
#define DEMO_MULTIPLE_SIMULATIONS_H_

#include <vector>
#include "biodynamo.h"

namespace bdm {

// Behavior that divides the agent at each time step
struct Divide : Behavior {
  BDM_BEHAVIOR_HEADER(Divide, Behavior, 1);

  Divide() {}

  void Run(Agent* agent) override { dynamic_cast<Cell*>(agent)->Divide(); }
};

inline int Simulate(int argc, const char** argv) {
  auto set_param = [](Param* param) {
    // Turn on export visualization
    param->export_visualization = true;
    param->visualize_agents["Cell"] = {};
  };

  // Create two simulations
  std::vector<Simulation*> simulations;
  simulations.push_back(new Simulation(argc, argv, set_param));
  simulations.push_back(new Simulation(argc, argv, set_param));

  // Initialize the model for each simulation
  for (auto* sim : simulations) {
    // If we switch between simulations we must call the function Activate();
    // Only one simulation can be active at any given time.
    sim->Activate();

    // Create initial model
    auto* rm = sim->GetResourceManager();
    Cell* cell = new Cell(30);
    cell->AddBehavior(new Divide());
    rm->AddAgent(cell);
  }

  // For each simulation simulate 5 timesteps
  for (auto* sim : simulations) {
    sim->Activate();
    sim->GetScheduler()->Simulate(5);
  }

  // Delete sinulations
  for (auto* sim : simulations) {
    sim->Activate();
    delete sim;
  }

  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm

#endif  // DEMO_MULTIPLE_SIMULATIONS_H_
