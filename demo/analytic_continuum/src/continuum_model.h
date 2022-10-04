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

#include <cmath>
#include "biodynamo.h"

#ifndef CONTINUUM_MODEL_H_
#define CONTINUUM_MODEL_H_

namespace bdm {

/// We define a custom analytic continuum model. We inherit form BioDynaMo's
/// base class `ScalarField` to obey the API. We implement the function:
/// f(x,y,z,t) = (1-e^{-t}) sin(w_x*x) * sin(w_y*y)
class AnalyticContinuum : public ScalarField {
 public:
  AnalyticContinuum() = default;
  explicit AnalyticContinuum(const TRootIOCtor *) {}
  ~AnalyticContinuum() final = default;

  void Initialize() final {}
  void Update() final {}

  /// This function is called in ever timestep of the simulation by the
  /// BioDynaMo core. It is used to compute update the state of the continuum
  /// model. Typically, this involves a numeric scheme that computes the
  /// solution of a differential equation. Here, the time dependence is
  /// completley characterized by the parameter `t` in f(x,y,z,t). Thus, we only
  /// need to make sure that `t` (`time_`) is updated..
  void Step(real_t dt) final { time_ += dt; }

  /// This function implements f(x,y,z,t) = (1-e^{-t}) sin(w_x*x) * sin(w_y*y).
  virtual real_t GetValue(const Real3 &position) const final {
    real_t time_scale_factor = 1.0 - std::exp(-time_);
    real_t w_x = 2 * Math::kPi / 50;
    real_t sin_factor_x = std::sin(position[0] * w_x);
    real_t w_y = 2 * Math::kPi / 100;
    real_t sin_factor_y = std::sin(position[1] * w_y);
    return time_scale_factor * sin_factor_x * sin_factor_y;
  }

  /// Our simulation does not not make use of the gradient information so we
  /// omit this function.
  virtual Real3 GetGradient(const Real3 &position) const final {
    // Not implemented
    Log::Warning("AnalyticContinuum::GetGradient() not implemented");
    return {0, 0, 0};
  };

  BDM_CLASS_DEF_OVERRIDE(AnalyticContinuum, 1);  // NOLINT

 private:
  real_t time_ = 0.0;
};

}  // namespace bdm

#endif  // CONTINUUM_MODEL_H_
