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
#include "biology_modules/inhibitation_module.h"
#include "biology_modules/physical_bond_module.h"
#include "biology_modules/random_walk_module.h"
#include "biology_modules/spring_force_module.h"
#include "biology_modules/stokes_velocity_module.h"
#include "core/parallel_execution/xml_util.h"
#include "core/substance_initializers.h"
#include "core/util/io.h"
#include "my_results.h"
#include "simulation_objects/monocyte.h"
#include "simulation_objects/t_cell.h"

#include "TH2I.h"

namespace bdm {

enum CellType { kMonocyte, kTCell };
enum Substances { kAntibody };

// Helper function to convert atomic containers to non-atomic
template <typename T>
void MakeNonAtomic(const std::vector<std::atomic<T>>& in, std::vector<T>* out) {
  out->resize(in.size());
  for (size_t i = 0; i < in.size(); i++) {
    (*out)[i] = in[i];
  }
}

inline int Simulate(int argc, const char** argv,
                    XMLParams* xml_params = nullptr) {
  Simulation simulation(argc, argv, xml_params);

  //////////////////////////////////////////////////////////////////////////////
  // Retrieve simulation parameters from XML parameter file
  //////////////////////////////////////////////////////////////////////////////
  auto xmlp = simulation.GetXMLParam();
  // xmlp.Print();

  // World parameters
  int timesteps = xmlp.Get("World", "timesteps");
  double min_space = xmlp.Get("World", "min_space");
  double max_space = xmlp.Get("World", "max_space");

  // T-Cell parameters
  int t_cell_population = xmlp.Get("T_Cell", "population");
  double t_cell_diameter = xmlp.Get("T_Cell", "diameter");
  double t_cell_walkspeed = xmlp.Get("T_Cell", "velocity");
  double t_cell_density = xmlp.Get("T_Cell", "mass_density");
  double t_cell_init_mean = xmlp.Get("T_Cell", "init_activation_mean");
  double t_cell_init_sigma = xmlp.Get("T_Cell", "init_activation_sigma");

  // Monocyte parameters
  int monocyte_population = xmlp.Get("Monocyte", "population");
  double monocyte_diameter = xmlp.Get("Monocyte", "diameter");
  double monocyte_density = xmlp.Get("Monocyte", "mass_density");

  // Antibody (substance) parameters
  double apd_amount = xmlp.Get("Antibody", "amount");
  double diff_rate = xmlp.Get("Antibody", "diffusion_rate");
  double decay_rate = xmlp.Get("Antibody", "decay_rate");
  double res = xmlp.Get("Antibody", "resolution");

  // Inhibition module parameters
  double inhib_sigma = xmlp.Get("Inhibition", "sigma");
  double inhib_mu = xmlp.Get("Inhibition", "mu");

  // StokesVelocity module parameters
  double stokes_u = xmlp.Get("StokesVelocity", "viscosity");
  double stokes_pf = xmlp.Get("StokesVelocity", "mass_density_fluid");

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize Monocytes
  //////////////////////////////////////////////////////////////////////////////
  auto mc_builder = [&](Double3 pos) {
    Monocyte* mc = new Monocyte(pos, monocyte_diameter, CellType::kMonocyte);
    mc->SetDensity(monocyte_density);
    mc->SetMaximumNumberOfSynapses(3);
    mc->AddBiologyModule(new RandomWalk(monocyte_diameter / 2));
    mc->AddBiologyModule(new StokesVelocity(stokes_u, stokes_pf));
    mc->AddBiologyModule(new Inhibitation(inhib_sigma, inhib_mu));
    return mc;
  };
  ModelInitializer::CreateCellsRandom(min_space, max_space, monocyte_population,
                                      mc_builder);

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize T-Cells
  //////////////////////////////////////////////////////////////////////////////
  auto tc_builder = [&](Double3 pos) {
    TCell* tc = new TCell(pos, t_cell_diameter, CellType::kTCell, timesteps);
    tc->SetDensity(t_cell_density);
    tc->SetInitialActivationIntensity(t_cell_init_mean, t_cell_init_sigma);
    tc->AddBiologyModule(new RandomWalk(t_cell_walkspeed));
    tc->AddBiologyModule(new StokesVelocity(stokes_u, stokes_pf));
    tc->AddBiologyModule(new ConnectWithinRadius(
        (0.75 * (t_cell_diameter + monocyte_diameter))));
    tc->AddBiologyModule(new PhysicalBond());
    return tc;
  };
  ModelInitializer::CreateCellsRandom(min_space, max_space, t_cell_population,
                                      tc_builder);

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize Anti-PD-1 substance
  //////////////////////////////////////////////////////////////////////////////
  ModelInitializer::DefineSubstance(Substances::kAntibody, "Antibody",
                                    diff_rate, decay_rate, res);
  ModelInitializer::InitializeSubstance(
      Substances::kAntibody, "Antibody",
      Uniform(min_space, max_space, apd_amount, Axis::kZAxis));

  //////////////////////////////////////////////////////////////////////////////
  // Schedule operation to obtain results of interest over time
  //////////////////////////////////////////////////////////////////////////////
  std::vector<std::atomic<int>> activity(timesteps);
  for (int i = 0; i < timesteps; i++) {
    activity[i] = 0;
  }
  auto* scheduler = simulation.GetScheduler();
  scheduler->AddOperation(Operation("Count activity", [&](SimObject* so) {
    uint64_t idx = scheduler->GetSimulatedSteps();
    if (auto* tcell = dynamic_cast<TCell*>(so)) {
      if (tcell->IsActivated()) {
        activity[idx]++;
      }
    }
  }));

  //////////////////////////////////////////////////////////////////////////////
  // Run the simulation
  //////////////////////////////////////////////////////////////////////////////
  simulation.GetScheduler()->Simulate(timesteps);

  // Collect histograms
  TH2I* merged_histo = nullptr;
  auto* rm = simulation.GetResourceManager();
  bool init = true;
  rm->ApplyOnAllElements([&](SimObject* so, SoHandle soh) {
    if (auto* tcell = dynamic_cast<TCell*>(so)) {
      if (init) {
        merged_histo = tcell->GetActivationHistogram();
        init = false;
      }
      merged_histo->Add(tcell->GetActivationHistogram());
    }
  });

  //////////////////////////////////////////////////////////////////////////////
  // Write my simulation results to file
  //////////////////////////////////////////////////////////////////////////////
  std::string name = "binding_cells";
  std::string brief = "Effect of Anti-PD-1 Concentration on T-Cell Activation";
  MyResults ex(name, brief);
  ex.initial_concentration = apd_amount;
  ex.activation_intensity = *merged_histo;
  MakeNonAtomic(activity, &(ex.activity));
  ex.WriteResultToROOT();

  return 0;
}

}  // namespace bdm

#endif  // SPRING_FORCE_H_
