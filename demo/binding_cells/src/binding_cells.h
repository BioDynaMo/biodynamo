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
#ifndef SPRING_FORCE_H_
#define SPRING_FORCE_H_

#include <atomic>

#include "biodynamo.h"
#include "biology_modules/connect_within_radius_module.h"
#include "biology_modules/constant_displacement_module.h"
#include "biology_modules/gravity_module.h"
#include "biology_modules/inhibitation_module.h"
#include "biology_modules/physical_bond_module.h"
#include "biology_modules/random_walk_module.h"
#include "biology_modules/spring_force_module.h"
#include "core/substance_initializers.h"
#include "core/util/io.h"
#include "my_experiment.h"
#include "plot_graph.h"
#include "simulation_objects/my_cell.h"

namespace bdm {

enum CellType { kMonocyte, kTCell };
enum Substances { kAntibody };

// Helper function to convert atomic containers to non-atomic
template <typename T>
void MakeNonAtomic(const std::vector<std::atomic<T>>& a, std::vector<T>* na) {
  na->resize(a.size());
  for (size_t i = 0; i < a.size(); i++) {
    (*na)[i] = a[i];
  }
}

inline int Simulate(int argc, const char** argv,
                    XMLParams* xml_params = nullptr) {
  Simulation simulation(argc, argv, xml_params);
  auto xmlp = simulation.GetXMLParam();

  // Simulation parameters
  // clang-format off
  int timesteps = xmlp.Get("World", "timesteps");
  double min_space = xmlp.Get("World", "min_space");
  double max_space = xmlp.Get("World", "max_space");
  int num_t_cells = xmlp.Get("T_Cell", "population");
  double t_cell_diameter = xmlp.Get("T_Cell", "diameter");
  int num_monocytes = xmlp.Get("Monocyte", "population");
  double monocyte_diameter = xmlp.Get("Monocyte", "diameter");
  double walk_speed = xmlp.Get("Monocyte", "velocity");
  double diff_rate = xmlp.Get("Antibody", "diffusion_rate");
  double decay_rate = xmlp.Get("Antibody", "decay_rate");
  double res = xmlp.Get("Antibody", "resolution");
  double br = xmlp.Get("ConnectWithinRadius", "binding_radius");
  double ct = xmlp.Get("Inhibition", "concentration_threshold");
  double bp = xmlp.Get("Inhibition", "binding_probability");
  // clang-format on

  // Create 2D layer of monocytes
  auto mc_builder = [&](Double3 position) {
    MyCell* cell = new MyCell(position, monocyte_diameter, CellType::kMonocyte);
    cell->AddBiologyModule(new RandomWalkXY(walk_speed));
    cell->AddBiologyModule(new Inhibitation(ct, bp));
    return cell;
  };
  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto* random = simulation.GetRandom();
  for (int i = 0; i < num_monocytes; i++) {
    double x = random->Uniform(min_space, min_space);
    double y = random->Uniform(min_space, max_space);
    double z = 0;
    auto* new_simulation_object = mc_builder({x, y, z});
    rm->push_back(new_simulation_object);
  }

  // Spawn T-Cells randomly
  auto tc_builder = [&](Double3 position) {
    MyCell* cell = new MyCell(position, t_cell_diameter, CellType::kTCell);
    cell->AddBiologyModule(new RandomWalk(2));
    cell->AddBiologyModule(new Gravity());
    cell->AddBiologyModule(new ConnectWithinRadius(br, CellType::kMonocyte));
    cell->AddBiologyModule(new PhysicalBond());
    return cell;
  };
  ModelInitializer::CreateCellsRandom(min_space, max_space, num_t_cells,
                                      tc_builder);

  // Create Antibody substance
  ModelInitializer::DefineSubstance(kAntibody, "Antibody", diff_rate,
                                    decay_rate, res);
  ModelInitializer::InitializeSubstance(kAntibody, "Antibody",
                                        GaussianBand(0, 5, Axis::kZAxis));

  // Schedule operation to obtain results of interest
  std::vector<int> t(timesteps);
  std::vector<std::atomic<int>> activity(timesteps);
  std::vector<std::atomic<int>> occupancy(timesteps);
  for (int i = 0; i < timesteps; i++) {
    activity[i] = num_t_cells;
    occupancy[i] = 0;
    t[i] = i;
  }
  auto* scheduler = simulation.GetScheduler();
  scheduler->AddOperation(Operation("Count activity", [&](SimObject* so) {
    uint64_t idx = scheduler->GetSimulatedSteps();
    if (auto* cell = bdm_static_cast<MyCell*>(so)) {
      if (cell->GetCellType() == 1 && cell->IsConnected()) {
        activity[idx]--;
      }
      if (cell->GetCellType() == 0 && cell->IsOccupied()) {
        occupancy[idx]++;
      }
    }
  }));

  // Run the simulation
  simulation.GetScheduler()->Simulate(timesteps);

  // Store my experimental results and write to file
  std::string name = "binding_cells";
  std::string brief = "T-Cell_Activity_Study";
  MyExperiment ex(name, brief);
  ex.timesteps = timesteps;
  MakeNonAtomic(activity, &(ex.activity));
  MakeNonAtomic(occupancy, &(ex.occupancy));
  ex.WriteResultToROOT();

  // PlotGraph(t, ex.result.activity, "activity_vs_time");
  // PlotGraph(t, ex.result.occupancy, "occupancy_vs_time");

  std::cout << "Simulation completed successfully!" << std::endl;
  return 0;
}

}  // namespace bdm

#endif  // SPRING_FORCE_H_
