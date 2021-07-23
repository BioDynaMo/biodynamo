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

#ifndef CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_PARTICLE_SWARM_PARAM_H_
#define CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_PARTICLE_SWARM_PARAM_H_

#include <string>

#include "core/multi_simulation/optimization_param_type/optimization_param_type.h"
#include "core/util/log.h"

namespace bdm {

/// A range of values
struct ParticleSwarmParam : public OptimizationParamType {
  ParticleSwarmParam() {}
  ParticleSwarmParam(const std::string& name, double min, double max, double iv)
      : OptimizationParamType(name),
        lower_bound(min),
        upper_bound(max),
        initial_value(iv) {
    Validate();
  };

  void Validate() const override {
    if (lower_bound > upper_bound) {
      Log::Fatal(
          "ParticleSwarmParam", "Tried to initialize parameter '", param_name,
          "' with a lower_bound value higher than upper_bound: ", lower_bound,
          " > ", upper_bound);
    }
  }

  OptimizationParamType* GetCopy() const override {
    return new ParticleSwarmParam(*this);
  }

  double GetValue(int n) const override {
    Log::Fatal("ParticleSwarmParam::GetValue",
               "Invalid operation! Values are obtained through the "
               "optimization library.");
    return 0.0;
  }

  uint32_t GetNumElements() const override {
    Log::Fatal("ParticleSwarmParam::GetValue", "Invalid operation!");
    return 0.0;
  }

  // The minimum value
  double lower_bound = 0;
  // THe maximum value
  double upper_bound = 0;
  // The stride
  double initial_value = 1;
  BDM_CLASS_DEF_OVERRIDE(ParticleSwarmParam, 1);
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_PARTICLE_SWARM_PARAM_H_
