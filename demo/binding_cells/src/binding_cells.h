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
#include "core/operation/operation_registry.h"
#include "core/operation/reduction_op.h"
#include "core/parallel_execution/util.h"
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
  int timesteps = 4000;
  double min_space = 0;
  double max_space = 200;

  // T-Cell parameters
  int t_cell_population = 2727;
  double t_cell_diameter = 0.9;
  double t_cell_walkspeed = 5;
  double t_cell_density = 1.077;
  double t_cell_init_mean = 3;
  double t_cell_init_sigma = 1;

  // Monocyte parameters
  int monocyte_population = 7272;
  double monocyte_diameter = 1.5;
  double monocyte_density = 1.067;

  // Antibody (substance) parameters
  double apd_amount = 10;
  double diff_rate = 0;
  double decay_rate = 0;
  double res = 10;

  // Inhibition module parameters
  double inhib_sigma = 1;
  double inhib_mu = -8.5;

  // StokesVelocity module parameters
  double stokes_u = .089;
  double stokes_pf = .997;
};

inline int Simulate(int argc, const char** argv,
                    Param* final_params = nullptr) {
  auto set_param = [&](Param* param) {
    param->Restore(std::move(*final_params));
  };
  Simulation simulation(argc, argv, set_param);

  // get a pointer to an instance of SimParam
  auto* sparam = simulation.GetParam()->GetModuleParam<SimParam>();

  std::cout << "Running simulation with monocyte_diameter = "
            << sparam->monocyte_diameter << std::endl;

  // T-Cells contain a TH2I histogram object that is not thread-safe by default,
  // so we need to enable ROOT's implicit multithreading awareness
  ROOT::EnableImplicitMT();

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize Monocytes
  //////////////////////////////////////////////////////////////////////////////
  auto mc_builder = [&](Double3 pos) {
    Monocyte* mc =
        new Monocyte(pos, sparam->monocyte_diameter, CellType::kMonocyte);
    mc->SetDensity(sparam->monocyte_density);
    mc->SetMaximumNumberOfSynapses(3);
    mc->AddBiologyModule(new RandomWalk(sparam->monocyte_diameter / 2));
    mc->AddBiologyModule(
        new StokesVelocity(sparam->stokes_u, sparam->stokes_pf));
    mc->AddBiologyModule(
        new Inhibitation(sparam->inhib_sigma, sparam->inhib_mu));
    return mc;
  };
  ModelInitializer::CreateCellsRandom(sparam->min_space, sparam->max_space,
                                      sparam->monocyte_population, mc_builder);

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize T-Cells
  //////////////////////////////////////////////////////////////////////////////
  auto tc_builder = [&](Double3 pos) {
    TCell* tc = new TCell(pos, sparam->t_cell_diameter, CellType::kTCell,
                          sparam->timesteps);
    tc->SetDensity(sparam->t_cell_density);
    tc->SetInitialActivationIntensity(sparam->t_cell_init_mean,
                                      sparam->t_cell_init_sigma);
    tc->AddBiologyModule(new RandomWalk(sparam->t_cell_walkspeed));
    tc->AddBiologyModule(
        new StokesVelocity(sparam->stokes_u, sparam->stokes_pf));
    tc->AddBiologyModule(new ConnectWithinRadius(
        (0.75 * (sparam->t_cell_diameter + sparam->monocyte_diameter))));
    tc->AddBiologyModule(new PhysicalBond());
    return tc;
  };
  ModelInitializer::CreateCellsRandom(sparam->min_space, sparam->max_space,
                                      sparam->t_cell_population, tc_builder);

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize Anti-PD-1 substance
  //////////////////////////////////////////////////////////////////////////////
  ModelInitializer::DefineSubstance(Substances::kAntibody, "Antibody",
                                    sparam->diff_rate, sparam->decay_rate,
                                    sparam->res);
  ModelInitializer::InitializeSubstance(
      Substances::kAntibody, "Antibody",
      Uniform(sparam->min_space, sparam->max_space, sparam->apd_amount,
              Axis::kZAxis));

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
  scheduler->Simulate(sparam->timesteps);

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
  ex.initial_concentration = sparam->apd_amount;
  ex.activation_intensity = *merged_histo;
  ex.activity = op_impl->results_;
  ex.WriteResultToROOT();

  return 0;
}

}  // namespace bdm

#endif  // SPRING_FORCE_H_
