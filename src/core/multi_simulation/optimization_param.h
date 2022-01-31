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

#ifndef CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_H_
#define CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_H_

#include "core/multi_simulation/optimization_param_type/optimization_param_type.h"
#include "core/param/param.h"
#include "core/param/param_group.h"

namespace bdm {

struct OptimizationParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(OptimizationParam, 1);

  OptimizationParam(const OptimizationParam& other) {
    this->params.resize(other.params.size());
    int i = 0;
    for (auto* param : other.params) {
      this->params[i] = param->GetCopy();
      i++;
    }
    this->algorithm = other.algorithm;
    this->repetition = other.repetition;
  }

  std::string algorithm;
  std::vector<OptimizationParamType*> params;
  // Number of times to repeat an experiment
  size_t repetition = 1;
  // Maximum number of optimization iterations
  size_t max_iterations = 100;
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_H_
