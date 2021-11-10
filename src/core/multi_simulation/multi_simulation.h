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

#ifndef CORE_MULTI_SIMULATION_MULTI_SIMULATION_H_
#define CORE_MULTI_SIMULATION_MULTI_SIMULATION_H_

#ifdef USE_MPI

#include <functional>
#include <string>

#include "core/analysis/time_series.h"
#include "core/param/param.h"

namespace bdm {
namespace experimental {

using experimental::TimeSeries;

// Signature of the Simulate() call
using TSimulate = std::function<void(int, const char**, TimeSeries*, Param*)>;

class MultiSimulation {
 public:
  MultiSimulation(int argc, const char** argv);

  MultiSimulation(int argc, const char** argv, const TimeSeries& real);

  ~MultiSimulation();

  void DeleteResultFiles(const std::string& dir);

  void MergeResultFiles(const std::string& dir);

  int Execute(const TSimulate& simulate_call);

 private:
  int argc_ = 0;
  const char** argv_ = nullptr;
  char** argv_copy_ = nullptr;
};

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_MULTI_SIMULATION_MULTI_SIMULATION_H_
