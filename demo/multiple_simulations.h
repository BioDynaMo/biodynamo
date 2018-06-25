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

#ifndef MULTIPLE_SIMULATIONS_H_
#define MULTIPLE_SIMULATIONS_H_

#include "biodynamo.h"
#include <vector>

namespace bdm {

/// BiologyModule that divides the simulation object at each time step
struct Divide : BaseBiologyModule {
  template <typename TSo>
  void Run(TSo* sim_object) {
    sim_object->Divide();
  }
};

template <typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  using BiologyModules = Variant<Divide>;
};

inline int Simulate(int argc, const char** argv) {
  // Create two simulations
  std::vector<Simulation<>*> simulations;
  simulations.push_back(new Simulation<>(argc, argv));
  // simulations.push_back(new Simulation<>(argc, argv));

  // Initialize the model for each simulation
  for (auto* sim : simulations) {
    // If we switch between simulations we must call the function Activate();
    // Only one simulation can be active at any given time.
    sim->Activate();

    auto* rm = sim->GetResourceManager();
    auto&& cell = rm->New<Cell>(30);
    cell.AddBiologyModule(Divide());
    rm->Get<Cell>()->Commit();

    auto* param = sim->GetParam();
    param->export_visualization_ = true;
    param->visualize_sim_objects_["Cell"] = {};
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

  return 0;
}

}  // namespace bdm

#endif  // MULTIPLE_SIMULATIONS_H_
