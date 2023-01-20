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

#include "core/model_initializer.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/diffusion/euler_depletion_grid.h"
#include "core/diffusion/euler_grid.h"
#include "core/diffusion/runge_kutta_grid.h"
#include "core/util/log.h"

namespace bdm {

void ModelInitializer::DefineSubstance(
    int substance_id, const std::string& substance_name, real_t diffusion_coeff,
    real_t decay_constant, int resolution,
    const std::vector<real_t>& binding_coefficients,
    const std::vector<int>& binding_substances) {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  auto* rm = sim->GetResourceManager();
  DiffusionGrid* dgrid = nullptr;
  if (decay_constant < 0.) {
    Log::Fatal("ModelInitializer::DefineSubstance",
               "The decay constant must be > 0 (", decay_constant, ").");
  }
  if (param->diffusion_method == "euler") {
    if (!binding_substances.empty()) {
      if (binding_coefficients.size() != binding_substances.size()) {
        Log::Fatal("ModelInitializer::DefineSubstance",
                   "The binding coefficients and substances have different",
                   " sizes (", binding_coefficients.size(), " vs ",
                   binding_substances.size(), ").");
      }
      for (auto binding_coeff : binding_coefficients) {
        if (binding_coeff < 0.) {
          Log::Fatal("ModelInitializer::DefineSubstance",
                     "The binding coefficients must be > 0 (", binding_coeff,
                     ").");
        }
      }

      dgrid = new EulerDepletionGrid(
          substance_id, substance_name, diffusion_coeff, decay_constant,
          resolution, binding_coefficients, binding_substances);
    } else {
      dgrid = new EulerGrid(substance_id, substance_name, diffusion_coeff,
                            decay_constant, resolution);
    }
  } else if (param->diffusion_method == "runge-kutta") {
    if (decay_constant != 0) {
      Log::Warning(
          "ModelInitializer::DefineSubstance",
          "RungeKuttaGrid does not support a decay constant. Using 0.");
    }
    if (!binding_substances.empty()) {
      Log::Warning("ModelInitializer::DefineSubstance",
                   "RungeKuttaGrid does not support depletion. Depleting "
                   "substances removed.");
    }
    dgrid = new RungeKuttaGrid(substance_id, substance_name, diffusion_coeff,
                               resolution);
  } else {
    Log::Error("ModelInitializer::DefineSubstance", "Diffusion method '",
               param->diffusion_method,
               "' does not exist. Defaulting to 'euler'");
    dgrid = new EulerGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution);
  }

  rm->AddContinuum(dgrid);
}

}  // namespace bdm
