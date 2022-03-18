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

#ifndef CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_PARTICLE_SWARM_PARAM_H_
#define CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_PARTICLE_SWARM_PARAM_H_

#include <string>

#include "core/multi_simulation/optimization_param_type/optimization_param_type.h"
#include "core/util/log.h"

namespace bdm {

/// A parameter type exclusively used for the Particle Swarm optimization
/// algorithm Defines an initial value, and the lower and upper bound
struct ParticleSwarmParam : public OptimizationParamType {
  ParticleSwarmParam() = default;
  ParticleSwarmParam(const std::string& name, real_t min, real_t max, real_t iv)
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

  real_t GetValue(int n) const override {
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
  real_t lower_bound = 0;
  // THe maximum value
  real_t upper_bound = 0;
  // The stride
  real_t initial_value = 1;
  BDM_CLASS_DEF_OVERRIDE(ParticleSwarmParam, 1);
};

}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_OPTIMIZATION_PARAM_TYPE_PARTICLE_SWARM_PARAM_H_
