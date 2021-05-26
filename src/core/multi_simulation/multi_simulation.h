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

#ifndef CORE_PARALLEL_EXECUTOR_H_
#define CORE_PARALLEL_EXECUTOR_H_

#ifdef USE_MPI

#include <functional>
#include <string>

#include "core/multi_simulation/result_data.h"
#include "core/param/param.h"

namespace bdm {

// Signature of the Simulate() call
using TSimulate = std::function<int(int, const char**, ResultData*, Param*)>;

class MultiSimulation {
 public:
  MultiSimulation(int argc, const char** argv);

  ~MultiSimulation();

  void DeleteResultFiles(const std::string& dir);

  void MergeResultFiles(const std::string& dir);

  int Execute(const TSimulate& simulate_call);

 private:
  int argc_ = 0;
  const char** argv_ = nullptr;
  char** argv_copy_ = nullptr;
};

}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_PARALLEL_EXECUTOR_H_
