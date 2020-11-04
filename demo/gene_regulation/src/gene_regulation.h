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

#ifndef DEMO_GENE_REGULATION_H_
#define DEMO_GENE_REGULATION_H_

#include <string>
#include <vector>

#include "biodynamo.h"

namespace bdm {

using std::array;
using std::vector;
using std::string;

inline int Simulate(int argc, const char** argv) {
  // Initialize BioDynaMo
  Simulation simulation(argc, argv);

  // Initialize RegulateGenes module.
  // To add functions to the module use RegulateGenes::AddGene() function.
  // You should pass to the function two variables.
  // The first is of type  std::function<double(double, double)>.
  // This is the function by which concentration of the protein will be
  // calculated.
  // The second is double. This is the initial value for the protein.
  RegulateGenes regulate_example;
  regulate_example.AddGene(
      [](double curr_time, double last_concentration) {
        return curr_time * last_concentration + 0.2f;
      },
      1);
  regulate_example.AddGene(
      [](double curr_time, double last_concentration) {
        return last_concentration * last_concentration * curr_time;
      },
      5);
  regulate_example.AddGene(
      [](double curr_time, double last_concentration) {
        return last_concentration + curr_time + 3;
      },
      7);

  // Define initial model -- in this example just one cell.
  auto construct = [&](const Double3& position) {
    Cell* cell = new Cell(position);
    cell->SetDiameter(30);
    cell->SetAdherence(0.4);
    cell->SetMass(1.0);
    cell->AddBiologyModule(regulate_example.GetCopy());
    return cell;
  };
  const std::vector<Double3>& positions = {{0, 0, 0}};
  ModelInitializer::CreateCells(positions, construct);

  // Run simulation
  auto* scheduler = simulation.GetScheduler();
  scheduler->Simulate(10);

  // Output concentration values for each gene
  auto* rm = simulation.GetResourceManager();
  auto* agent = rm->GetAgent(AgentUid(0));
  const auto* first_bm = agent->GetAllBiologyModules()[0];
  auto* regulate_genes = dynamic_cast<const RegulateGenes*>(first_bm);
  const auto& concentrations = regulate_genes->GetConcentrations();
  std::cout << "Gene concentrations after " << scheduler->GetSimulatedSteps()
            << " time steps" << std::endl;
  for (double concentration : concentrations) {
    std::cout << concentration << std::endl;
  }

  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm

#endif  // DEMO_GENE_REGULATION_H_
