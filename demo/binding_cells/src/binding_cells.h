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
#include "core/operation/operation.h"
#include "core/operation/reduction_op.h"
#include "core/operation/operation_registry.h"
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

// Parameters specific for this simulation
struct SimParam : public ModuleParam {
  BDM_MODULE_PARAM_HEADER(SimParam, 1);

  // World parameters
  int timesteps = 0;
  double min_space = 0;
  double max_space = 0;

  // T-Cell parameters
  int t_cell_population = 0;
  double t_cell_diameter = 0;
  double t_cell_walkspeed = 0;
  double t_cell_density = 0;
  double t_cell_init_mean = 0;
  double t_cell_init_sigma = 0;

  // Monocyte parameters
  int monocyte_population = 0;
  double monocyte_diameter = 0;
  double monocyte_density = 0;

  // Antibody (substance) parameters
  double apd_amount = 0;
  double diff_rate = 0;
  double decay_rate = 0;
  double res = 0;

  // Inhibition module parameters
  double inhib_sigma = 0;
  double inhib_mu = 0;

  // StokesVelocity module parameters
  double stokes_u = 0;
  double stokes_pf = 0;
};

inline int Simulate(int argc, const char** argv,
                    XMLParams* xml_params = nullptr) {
  Param::RegisterModuleParam(new SimParam());
  Simulation simulation(argc, argv, xml_params);

  // get a pointer to the param object
  auto* param = simulation.GetParam();
  // get a pointer to an instance of SimParam
  auto* sparam = param->GetModuleParam<SimParam>();

  // T-Cells contain a TH2I histogram object that is not thread-safe by default,
  // so we need to enable ROOT's implicit multithreading awareness
  ROOT::EnableImplicitMT();

  // World parameters
  int timesteps = sparam->timesteps;
  double min_space = sparam->min_space;
  double max_space = sparam->max_space;

  // T-Cell parameters
  int t_cell_population = sparam->t_cell_population;
  double t_cell_diameter = sparam->t_cell_diameter;
  double t_cell_walkspeed = sparam->t_cell_walkspeed;
  double t_cell_density = sparam->t_cell_density;
  double t_cell_init_mean = sparam->t_cell_init_mean;
  double t_cell_init_sigma = sparam->t_cell_init_sigma;

  // Monocyte parameters
  int monocyte_population = sparam->monocyte_population;
  double monocyte_diameter = sparam->monocyte_diameter;
  double monocyte_density = sparam->monocyte_density;

  // Antibody (substance) parameters
  double apd_amount = sparam->apd_amount;
  double diff_rate = sparam->diff_rate;
  double decay_rate = sparam->decay_rate;
  double res = sparam->res;

  // Inhibition module parameters
  double inhib_sigma = sparam->inhib_sigma;
  double inhib_mu = sparam->inhib_mu;

  // StokesVelocity module parameters
  double stokes_u = sparam->stokes_u;
  double stokes_pf = sparam->stokes_pf;

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
  struct CheckActivity : public Functor<void, SimObject*, int*> {
    void operator()(SimObject* so, int* tl_result) {
      if (auto* tcell = dynamic_cast<TCell*>(so)) {
        if (tcell->IsActivated()) {
          (*tl_result)++;
        }
      }
    }
  };

  auto* scheduler = simulation.GetScheduler();
  auto* op = NewOperation("ReductionOpInt");
  auto* op_impl = op->GetImplementation<ReductionOp<int>>();
  op_impl->Initialize(new CheckActivity(), new SumReduction<int>());
  scheduler->ScheduleOp(op);

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
  ex.activity = op_impl->results_;
  ex.WriteResultToROOT();

  return 0;
}

}  // namespace bdm

#endif  // SPRING_FORCE_H_
