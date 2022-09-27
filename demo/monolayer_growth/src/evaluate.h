// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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
  auto get_env_dims = [](Simulation* sim) {
    auto* env = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
    double env_dim_x, env_dim_y;
    env_dim_x = env->GetDimensions()[1] - env->GetDimensions()[0];
    env_dim_y = env->GetDimensions()[3] - env->GetDimensions()[2];
    return std::max(env_dim_x, env_dim_y);
  };
  auto get_time = [](Simulation* sim) {
    auto* scheduler = sim->GetScheduler();
    auto* sparam =
        sim->GetParam()
            ->Get<SimParam>();  // get a pointer to an instance of SimParam
    return (double)(sparam->t0 + scheduler->GetSimulatedSteps() *
                                     sparam->step_length / 24.);  // /24 -> days
  };

  ts->AddCollector("env_dims", get_env_dims, get_time);
}

inline void PrintResults(const std::vector<TimeSeries>& individual_rd,
                         const std::string& folder,
                         const bool plot_legend = true,
                         const std::string& filename = "result") {
  TimeSeries allts;
  std::vector<double> times, sizes;

  int i = 0;
  for (auto& ind_ts : individual_rd) {
    allts.Add(ind_ts, Concat("i", i++));
    times = ind_ts.GetXValues("env_dims");
    sizes = ind_ts.GetYValues("env_dims");
  }

  // Add exp data from Figure 1 from Drasdo and Hoehme (2005)
  allts.Add("experimental_data",
            {336 / 24., 386 / 24., 408 / 24., 481 / 24., 506 / 24., 646 / 24.},
            {1140, 1400, 1590, 2040, 2250, 3040});

  LineGraph lg(&allts, "", "Time [days]", "2D Monolayer size (um)", plot_legend,
               nullptr, 350, 250);

  for (uint64_t i = 0; i < individual_rd.size(); ++i) {
    lg.Add(Concat("env_dims-i", i), "Sim data ", "LP", kBlue, 0.2, kSolid, 2,
           kBlue, 0.7, kFullCircle, 0.5);
  }

  // Add style for exp data
  lg.Add("experimental_data", "Exp data ", "LP", kBlack, 0.2, kSolid, 2, kBlack,
         0.7, kFullCircle, 0.5);

  if (plot_legend) {
    lg.SetLegendPosNDC(0.1, 0.7, 0.3, 0.9);
  }

  // Export monolayer size to csv file
  std::ofstream file;
  if (!file.is_open()) {
    file.open("monolayer_diam.csv");
  }

  for (size_t i = 0; i < times.size(); i++) {
    file << (int)times[i] << "\t " << sizes[i] << std::endl;
  }

  file.close();

  // lg.GetTMultiGraph()->SetMinimum(0.);
  lg.SaveAs(Concat(folder, "/", filename), {".svg"});
}

}  // namespace bdm

#endif  // EVALUATE_H_
