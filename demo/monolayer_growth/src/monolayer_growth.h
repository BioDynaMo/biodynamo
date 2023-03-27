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

#ifndef SPHERE_H_
#define SPHERE_H_

#include "behaviours.h"
#include "biodynamo.h"
#include "cell_cell_force.h"
#include "core/environment/uniform_grid_environment.h"
#include "core/interaction_force.h"
#include "core/operation/mechanical_forces_op.h"
#include "custom_ops.h"
#include "cycling_cell.h"
#include "evaluate.h"
#include "sim_param.h"

namespace bdm {

using experimental::TimeSeries;

inline int Simulate(int argc, const char** argv) {
  // Adding space edge of but to be used in larger use case.
  auto set_param = [](Param* param) {
    param->use_progress_bar = true;
    param->bound_space = Param::BoundSpaceMode::kOpen;
    param->min_bound = -2000;
    param->max_bound = 2000;
    param->export_visualization = false;
    param->visualization_interval = 500;
    param->visualize_agents["CyclingCell"] = {};
    param->statistics = false;
    param->simulation_time_step = 0.1;
    param->simulation_max_displacement = 100;  // 3 is the default value
  };

  // Before we create a simulation we have to tell BioDynaMo about
  // the new parameters.
  Param::RegisterParamGroup(new SimParam());

  // Create a new simulation
  Simulation simulation(argc, argv, set_param);
  SetupResultCollection(&simulation);
  auto* ctxt = simulation.GetExecutionContext();  // Get the execution context
  auto* scheduler = simulation.GetScheduler();    // Get the scheduler
  auto* param = simulation.GetParam();            // Get the parameters
  const auto* sparam = param->Get<SimParam>();  // Get the simulation parameters

  real_t x_coord = sparam->pos0;
  real_t y_coord;

  while (x_coord < sparam->posN) {
    y_coord = sparam->pos0;
    while (y_coord < sparam->posN) {
      CyclingCell* cell = new CyclingCell({x_coord, y_coord, 0});

      cell->SetDiameter(sparam->cell_diam);
      cell->SetVinit(cell->GetVolume());
      cell->SetVmax(cell->GetVolume() * 2.0);
      cell->SetDeltaT(0.0);
      cell->SetCycle(CellState::kG1);
      cell->SetCanDivide(true);
      cell->AddBehavior(new GrowthAndCellCycle());
      ctxt->AddAgent(cell);  // put the created cell in our cells structure
      y_coord += sparam->cell_diam;
    }
    x_coord += sparam->cell_diam;
  }

  // Set the box length
  auto* env =
      dynamic_cast<UniformGridEnvironment*>(simulation.GetEnvironment());
  env->SetBoxLength(sparam->cell_diam * 2);

  // Check if the cells have moved along the z direction and, if so, move them
  // back
  auto* move_cells_back = NewOperation("move_cells_plane");
  simulation.GetScheduler()->ScheduleOp(move_cells_back);

  // Set time series freq (will measure the size of the uniform grid
  // environment)
  auto* updatetimeseries_op = scheduler->GetOps("update time series")[0];
  updatetimeseries_op->frequency_ = sparam->ts_freq;

  // Custom force module
  auto* custom_force = new CellCellForce();
  auto* mech_op = scheduler->GetOps("mechanical forces")[0];
  auto* force_implementation = mech_op->GetImplementation<MechanicalForcesOp>();
  force_implementation->SetInteractionForce(custom_force);
  mech_op->frequency_ = static_cast<size_t>(param->simulation_time_step / 0.01);

  // Run simulation
  size_t time_steps =
      static_cast<size_t>(sparam->total_time / param->simulation_time_step);

  simulation.GetScheduler()->Simulate(time_steps);
  std::cout << "Simulation completed successfully!" << std::endl;

  // Export results (images and data)
  ExportResults();

  return 0;
}

}  // namespace bdm

#endif  // SPHERE_H_
