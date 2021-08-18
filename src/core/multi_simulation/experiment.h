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
#include "core/multi_simulation/database.h"
#include "core/param/param.h"

namespace bdm {
namespace experimental {

inline double Experiment(
    const std::function<void(Param*, TimeSeries*)>& dispatch_to_worker,
    Param* param, size_t iterations) {
  // Run the simulation with the input parameters for N iterations
  std::vector<TimeSeries> results(iterations);
  for (size_t i = 0; i < iterations; i++) {
    Param param_copy = *param;
    dispatch_to_worker(&param_copy, &results[i]);
  }

  // Compute the mean result values of the N iterations
  TimeSeries simulated;
  TimeSeries::Merge(&simulated, results,
                    [](const std::vector<double> all_y_values, double* y,
                       double* eh, double* el) {
                      *y =
                          TMath::Mean(all_y_values.begin(), all_y_values.end());
                    });

  // Extract analytical / experimental values from the database
  TimeSeries real = Database::GetInstance()->data_;

  // Compute and return the error between the real and simulated data
  double err = TimeSeries::ComputeError(real, simulated);

  return err;
}

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_EXPERIMENT_H_
