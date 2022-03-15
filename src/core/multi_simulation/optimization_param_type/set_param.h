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

#ifndef CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_SET_PARAM_H_
#define CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_SET_PARAM_H_

#include <string>
#include <vector>

#include "core/multi_simulation/optimization_param_type/optimization_param_type.h"

namespace bdm {

/// A set of values (e.g. [-12, 3.2, 43, 98])
/// All values are interpreted as real precision floating point types
struct SetParam : public OptimizationParamType {
  SetParam() {}
  SetParam(const std::string& name, const std::vector<real> v)
      : OptimizationParamType(name), values(v) {}

  OptimizationParamType* GetCopy() const override {
    return new SetParam(*this);
  }

  size_t size() const { return values.size(); }
  real at(size_t n) const { return values.at(n); }

  uint32_t GetNumElements() const override { return this->size(); }
  real GetValue(int n) const override { return this->at(n); }

  std::vector<real> values;
  BDM_CLASS_DEF_OVERRIDE(SetParam, 1);
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_SET_PARAM_H_
