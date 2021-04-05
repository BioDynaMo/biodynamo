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

#ifndef CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_H_
#define CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_H_

#include <functional>
#include <string>

#include "core/multi_simulation/optimization_param.h"
#include "core/param/param.h"

namespace bdm {

class MultiSimulationManager;

struct Algorithm {
  virtual ~Algorithm() {}

  virtual void operator()(
      const std::function<void(Param*)>& send_params_to_worker) = 0;

  OptimizationParam* opt_params_;
  Param* default_params_;
  MultiSimulationManager* msm_ = nullptr;
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_ALGORITHM_ALGORITHM_H_
