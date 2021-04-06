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

#include "agents/monocyte.h"
#include "agents/t_cell.h"
#include "biodynamo.h"
#include "biology_modules/connect_within_radius_module.h"
#include "biology_modules/constant_displacement_module.h"
#include "biology_modules/inhibitation_module.h"
#include "biology_modules/physical_bond_module.h"
#include "biology_modules/random_walk_module.h"
#include "biology_modules/spring_force_module.h"
#include "biology_modules/stokes_velocity_module.h"
#include "core/multi_simulation/util.h"
#include "core/multi_simulation/error_matrix.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/operation/reduction_op.h"
#include "core/substance_initializers.h"
#include "core/util/io.h"
#include "my_results.h"

#include "TH2I.h"
#include "TROOT.h"

namespace bdm {

enum CellType { kMonocyte, kTCell };
enum Substances { kAntibody };

// Parameters specific for this simulation
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  // World parameters
  int timesteps = 40;
  double min_space = 0;
  double max_space = 20;

  // T-Cell parameters
  int t_cell_population = 272;
  double t_cell_diameter = 0.9;
  double t_cell_walkspeed = 5;
  double t_cell_density = 1.077;
  double t_cell_init_mean = 3;
  double t_cell_init_sigma = 1;

  // Monocyte parameters
  int monocyte_population = 727;
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

inline double Simulate(int argc, const char** argv,
                       Param* final_params = nullptr) {
  auto set_param = [&](Param* param) {
    param->Restore(std::move(*final_params));
  };
  Simulation simulation(argc, argv, set_param);

  // get a pointer to an instance of SimParam
  auto* sparam = simulation.GetParam()->Get<SimParam>();

  // T-Cells contain a TH2I histogram object that is not thread-safe by default,
  // agent we need to enable ROOT's implicit multithreading awareness
  ROOT::EnableImplicitMT();

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize Monocytes
  //////////////////////////////////////////////////////////////////////////////
  auto mc_builder = [&](Double3 pos) {
    Monocyte* mc =
        new Monocyte(pos, sparam->monocyte_diameter, CellType::kMonocyte);
    mc->SetDensity(sparam->monocyte_density);
    mc->SetMaximumNumberOfSynapses(3);
    mc->AddBehavior(new RandomWalk(sparam->monocyte_diameter / 2));
    mc->AddBehavior(new StokesVelocity(sparam->stokes_u, sparam->stokes_pf));
    mc->AddBehavior(new Inhibitation(sparam->inhib_sigma, sparam->inhib_mu));
    return mc;
  };
  ModelInitializer::CreateAgentsRandom(sparam->min_space, sparam->max_space,
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
    tc->AddBehavior(new RandomWalk(sparam->t_cell_walkspeed));
    tc->AddBehavior(new StokesVelocity(sparam->stokes_u, sparam->stokes_pf));
    tc->AddBehavior(new ConnectWithinRadius(
        (0.75 * (sparam->t_cell_diameter + sparam->monocyte_diameter))));
    tc->AddBehavior(new PhysicalBond());
    return tc;
  };
  ModelInitializer::CreateAgentsRandom(sparam->min_space, sparam->max_space,
                                       sparam->t_cell_population, tc_builder);

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize Anti-PD-1 substance
  //////////////////////////////////////////////////////////////////////////////
  ModelInitializer::DefineSubstance(Substances::kAntibody, "Antibody",
                                    sparam->diff_rate, sparam->decay_rate,
                                    sparam->res);
  ModelInitializer::InitializeSubstance(
      Substances::kAntibody, Uniform(sparam->min_space, sparam->max_space,
                                     sparam->apd_amount, Axis::kZAxis));

  //////////////////////////////////////////////////////////////////////////////
  // Schedule operation to obtain results of interest over time
  //////////////////////////////////////////////////////////////////////////////
  struct CheckActivity : public Functor<void, Agent*, int*> {
    void operator()(Agent* agent, int* tl_result) {
      if (auto* tcell = dynamic_cast<TCell*>(agent)) {
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
  rm->ForEachAgent([&](Agent* agent, AgentHandle ah) {
    if (auto* tcell = dynamic_cast<TCell*>(agent)) {
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
  ex.activity = op_impl->GetResults();
  ex.WriteResultToROOT();

  //////////////////////////////////////////////////////////////////////////////
  // Compute error for optimization feedback
  //////////////////////////////////////////////////////////////////////////////
  auto final_activity = *(ex.activity.end());
  auto expected_val =
      simulation.GetParam()->Get<OptimizationParam>()->expected_val;

  SquaredError se;
  auto error = se.Compute(final_activity, expected_val);

  return error;
}

}  // namespace bdm

#endif  // SPRING_FORCE_H_
