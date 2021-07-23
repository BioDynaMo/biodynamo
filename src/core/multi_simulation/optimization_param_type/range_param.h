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

#ifndef CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_RANGE_PARAM_H_
#define CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_RANGE_PARAM_H_

#include <string>

#include "core/multi_simulation/optimization_param_type/optimization_param_type.h"
#include "core/util/log.h"

namespace bdm {

/// A range of values
struct RangeParam : public OptimizationParamType {
  RangeParam() {}
  RangeParam(const std::string& name, double min, double max, double stride)
      : OptimizationParamType(name),
        lower_bound(min),
        upper_bound(max),
        stride(stride) {
    Validate();
    num_elements = std::round(((upper_bound - lower_bound) + stride) / stride);
  };

  void Validate() const override {
    if (lower_bound > upper_bound) {
      Log::Fatal("RangeParam", "Tried to initialize parameter '", param_name,
                 "' with a lower_bound value higher than upper_bound: ",
                 lower_bound, " > ", upper_bound);
    }
  }

  OptimizationParamType* GetCopy() const override {
    return new RangeParam(*this);
  }

  // Get the nth value
  double GetValue(int n) const override {
    double curr = lower_bound + n * stride;
    return curr > upper_bound ? upper_bound : curr;
  }

  // Returns the number of discrete values that this range contains (including
  // the `lower_bound` and `upper_bound` values)
  uint32_t GetNumElements() const override { return num_elements; }

  // The minimum value
  double lower_bound = 0;
  // THe maximum value
  double upper_bound = 0;
  // The stride
  double stride = 1;
  // Cache the number of elements that is covered by this range
  uint32_t num_elements;
  BDM_CLASS_DEF_OVERRIDE(RangeParam, 1);
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_RANGE_PARAM_H_
