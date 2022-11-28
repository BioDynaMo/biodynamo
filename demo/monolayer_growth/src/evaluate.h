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

#ifndef EVALUATE_H_
#define EVALUATE_H_

#include <TAxis.h>
#include <TLegend.h>
#include <TMultiGraph.h>
#include <cmath>
#include <vector>
#include "biodynamo.h"
#include "sim_param.h"

using namespace bdm::experimental;

namespace bdm {

inline void SetupResultCollection(Simulation* sim) {
  auto* ts = sim->GetTimeSeries();

  // Monolayer size
  auto get_env_dims = [](Simulation* sim) {
    auto* env = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
    real_t env_dim_x, env_dim_y;
    env_dim_x = env->GetDimensions()[1] - env->GetDimensions()[0];
    env_dim_y = env->GetDimensions()[3] - env->GetDimensions()[2];
    return std::max(env_dim_x, env_dim_y);
  };

  // Number of cells
  auto get_num_cells = [](Simulation* sim) {
    auto* rm = sim->GetResourceManager();
    return static_cast<real_t>(rm->GetNumAgents());
  };

  // Time in days
  auto get_time = [](Simulation* sim) {
    auto* scheduler = sim->GetScheduler();
    const auto* param = sim->GetParam();
    const auto* sparam = param->Get<SimParam>();
    return (real_t)(sparam->t0 + scheduler->GetSimulatedTime() / 24.);
  };
  ts->AddCollector("total_cells", get_num_cells, get_time);
  ts->AddCollector("env_dims", get_env_dims, get_time);
}

inline void ExportResults(const bool plot_legend = true,
                          const std::string& filename = "result") {
  // Prerequisites
  const std::string folder = Simulation::GetActive()->GetOutputDir();
  auto* ts = Simulation::GetActive()->GetTimeSeries();
  TimeSeries allts;
  std::vector<real_t> times, sizes;

  // Add simulated data
  allts.Add(*ts, Concat("i", 0));
  times = ts->GetXValues("env_dims");
  sizes = ts->GetYValues("env_dims");

  // Add experimental data from Figure 1 from Drasdo and Hoehme (2005)
  allts.Add("experimental_data",
            {336 / 24., 386 / 24., 408 / 24., 481 / 24., 506 / 24., 646 / 24.},
            {1140, 1400, 1590, 2040, 2250, 3040});

  // Initialize line graph
  LineGraph lg(&allts, "", "Time [days]", "2D Monolayer size (um)", plot_legend,
               nullptr, 350, 250);

  // Add simulated data
  lg.Add(Concat("env_dims-i", 0), "Sim data ", "LP", kBlue, 0.2, kSolid, 2,
         kBlue, 0.7, kFullCircle, 0.5);

  // Add style for exp data
  lg.Add("experimental_data", "Exp data ", "LP", kBlack, 0.2, kSolid, 2, kBlack,
         0.7, kFullCircle, 0.5);

  // Add legend
  if (plot_legend) {
    lg.SetLegendPosNDC(0.1, 0.7, 0.3, 0.9);
  }

  // Save plot
  lg.SaveAs(Concat(folder, "/", filename), {".svg", ".png"});

  // Save data
  ts->SaveJson(Concat(folder, "/monolayer_growth.json"));
}

}  // namespace bdm

#endif  // EVALUATE_H_
