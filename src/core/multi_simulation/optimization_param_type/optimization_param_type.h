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

#ifndef CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_OPTIMIZATION_PARAM_TYPE_H_
#define CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_OPTIMIZATION_PARAM_TYPE_H_

#include <string>
#include <vector>

#include "core/util/io.h"

namespace bdm {

/// An interface for creating different types of optimization parameters
struct OptimizationParamType {
  OptimizationParamType() {}
  explicit OptimizationParamType(const std::string& name) : param_name(name) {}
  virtual ~OptimizationParamType() {}

  virtual OptimizationParamType* GetCopy() const = 0;

  virtual uint32_t GetNumElements() const = 0;
  virtual double GetValue(int n) const = 0;
  virtual void Validate() const {};

  // Return the substring before the last "::", which should be
  // bdm::<ParamGroup>
  std::string GetGroupName() {
    size_t found = param_name.find_last_of("::");
    return param_name.substr(0, found - 1);
  }

  // Return the substring after the last "::", which should be <param_name>
  std::string GetParamName() {
    size_t found = param_name.find_last_of("::");
    return param_name.substr(found + 1);
  }

  // Must be in format bdm::<ParamGroup>::<param_name>
  std::string param_name;
  BDM_CLASS_DEF(OptimizationParamType, 1);
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_OPTIMIZATION_PARAM_TYPE_H_
