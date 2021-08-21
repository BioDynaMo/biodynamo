// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_MULTI_SIMULATION_EXPERIMENT_H_
#define CORE_MULTI_SIMULATION_EXPERIMENT_H_

#include <functional>
#include <vector>

#include "TMath.h"

#include "core/analysis/time_series.h"
#include "core/functor.h"
#include "core/multi_simulation/database.h"
#include "core/param/param.h"

namespace bdm {
namespace experimental {

// Runs the given `simulation` for `iterations` amount of times` and computes
// the mean of the simulated results. If a real (experimental / analytical)
// dataset is presented (either as the argument or through a database), we
// compute the average error and return it
inline double Experiment(
    Functor<void, Param*, TimeSeries*>& simulation, size_t iterations,
    const Param* param, TimeSeries* real = nullptr,
    Functor<void, const std::vector<TimeSeries>&, const TimeSeries&,
            const TimeSeries&>* post_simulation = nullptr) {
  // If no experimental / analytical data is given, we try to extract it from
  // the database
  bool use_real_data = true;
  if (!real) {
    real = &(Database::GetInstance()->data_);
    // If also no real data is present in the database, we just run the
    // simulation
    if (!real) {
      use_real_data = false;
    }
  }

  // Run the simulation with the input parameters for N iterations
  std::vector<TimeSeries> results(iterations);
  for (size_t i = 0; i < iterations; i++) {
    Param param_copy = *param;
    simulation(&param_copy, &results[i]);
  }

  // Compute the mean result values of the N iterations
  TimeSeries simulated;
  TimeSeries::Merge(&simulated, results,
                    [](const std::vector<double> all_y_values, double* y,
                       double* eh, double* el) {
                      *y =
                          TMath::Mean(all_y_values.begin(), all_y_values.end());
                    });

  // Execute post-simulation lambda (e.g. plotting or exporting of simulated
  // data)
  if (post_simulation) {
    (*post_simulation)(results, simulated, *real);
  }

  if (use_real_data) {
    // Compute and return the error between the real and simulated data
    double err = TimeSeries::ComputeError(*real, simulated);

    return err;
  }
  return 0.0;
}

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_EXPERIMENT_H_
