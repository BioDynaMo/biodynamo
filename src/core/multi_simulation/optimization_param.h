// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
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

#include "core/multi_simulation/util.h"
#include "core/param/param_group.h"

namespace bdm {

struct OptimizationParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(OptimizationParam, 1);

  OptimizationParam(const OptimizationParam& other) {
    this->params_.resize(other.params_.size());
    int i = 0;
    for (auto* param : other.params_) {
      this->params_[i] = param->GetCopy();
      i++;
    }
    this->algorithm_ = other.algorithm_;
  }

  std::string algorithm_;
  std::vector<Container*> params_;
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_H_
