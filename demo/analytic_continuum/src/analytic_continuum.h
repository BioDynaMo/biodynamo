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
#ifndef ANALYTIC_CONTINUUM_H_
#define ANALYTIC_CONTINUUM_H_

#include "biodynamo.h"
#include "continuum_model.h"
#include "my_agent.h"
#include "my_behavior.h"

namespace bdm {

inline int Simulate(int argc, const char** argv) {
  // Define the name and id of the custom analytic continuum.
  int continuum_id = 0;
  std::string continuum_name = "AnalyticContinuum";

  // Initialize the simulation object. Note that the file `bdm.json` is parsed
  // automatically and influences the simulation parameter.
  Simulation simulation(argc, argv);

  // Define function to create an agent.
  auto create_agent = [](const Double3& position) {
    auto* agent = new ContinuumRetrieverAgent(position);
    agent->SetDiameter(1.0);
    // Add a behavior to the agent that allows it to sense the continuum value.
    agent->AddBehavior(new RetrieveContinuumValue());
    return agent;
  };

  // Use the ModelInitializer to distribute agents in a 3D grid (cube).
  uint64_t agents_per_dim = 50;
  double space_between_agents = 5;
  ModelInitializer::Grid3D(agents_per_dim, space_between_agents, create_agent);

  // Add a new continuum model to the simulation.
  auto* ac = new AnalyticContinuum();
  ac->SetContinuumId(continuum_id);
  ac->SetContinuumName(continuum_name);
  auto* rm = simulation.GetResourceManager();
  rm->AddContinuum(ac);

  // Run simulation with roughly 100000 agents for 500 timestep
  simulation.GetScheduler()->Simulate(500);

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // ANALYTIC_CONTINUUM_H_
