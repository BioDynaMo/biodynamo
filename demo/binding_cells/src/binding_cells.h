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
#ifndef BINDING_CELLS_H_
#define BINDING_CELLS_H_

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
#include "core/analysis/time_series.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/operation/reduction_op.h"
#include "core/substance_initializers.h"
#include "core/util/io.h"

#include "TH2I.h"
#include "TROOT.h"

namespace bdm {

using experimental::TimeSeries;

enum CellType { kMonocyte, kTCell };
enum Substances { kAntibody };

// Parameters specific for this simulation
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  // World parameters
  int timesteps = 40;
  real_t min_space = 0;
  real_t max_space = 20;

  // T-Cell parameters
  int t_cell_population = 272;
  real_t t_cell_diameter = 0.9;
  real_t t_cell_walkspeed = 5;
  real_t t_cell_density = 1.077;
  real_t t_cell_init_mean = 3;
  real_t t_cell_init_sigma = 1;

  // Monocyte parameters
  int monocyte_population = 727;
  real_t monocyte_diameter = 1.5;
  real_t monocyte_density = 1.067;

  // Antibody (substance) parameters
  real_t apd_amount = 10;
  real_t diff_rate = 0;
  real_t decay_rate = 0;
  real_t res = 10;

  // Inhibition module parameters
  real_t inhib_sigma = 1;
  real_t inhib_mu = -8.5;

  // StokesVelocity module parameters
  real_t stokes_u = .089;
  real_t stokes_pf = .997;
};

inline void Simulate(int argc, const char** argv, TimeSeries* result,
                     Param* final_params = nullptr) {
  auto set_param = [&](Param* param) {
    param->Restore(std::move(*final_params));
  };
  Simulation simulation(argc, argv, set_param);

  // Get a pointer to an instance of SimParam
  auto* sparam = simulation.GetParam()->Get<SimParam>();

  // T-Cells contain a TH2I histogram object that is not thread-safe by default,
  // agent we need to enable ROOT's implicit multithreading awareness
  ROOT::EnableImplicitMT();

  //////////////////////////////////////////////////////////////////////////////
  // Create and initialize Monocytes
  //////////////////////////////////////////////////////////////////////////////
  auto mc_builder = [&](Real3 pos) {
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
  auto tc_builder = [&](Real3 pos) {
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
  // Collect results of interest
  //////////////////////////////////////////////////////////////////////////////
  auto* ts = simulation.GetTimeSeries();
  auto activated = [](Simulation* sim) {
    auto condition = L2F([](Agent* a) {
      if (auto tcell = dynamic_cast<TCell*>(a)) {
        return tcell->IsActivated();
      }
      return false;
    });
    auto result = static_cast<real_t>(bdm::experimental::Count(sim, condition));
    auto num_agents = sim->GetResourceManager()->GetNumAgents();
    return result / static_cast<real_t>(num_agents);
  };
  ts->AddCollector("activated", activated);

  //////////////////////////////////////////////////////////////////////////////
  // Run the simulation
  //////////////////////////////////////////////////////////////////////////////
  simulation.Simulate(sparam->timesteps);

  std::cout << "Simulation completed successfully!" << std::endl;

  //////////////////////////////////////////////////////////////////////////////
  // Return simulation results
  //////////////////////////////////////////////////////////////////////////////
  *result = *simulation.GetTimeSeries();
}

}  // namespace bdm

#endif  // BINDING_CELLS_H_
