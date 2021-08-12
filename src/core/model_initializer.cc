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

#include "core/model_initializer.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/diffusion/euler_grid.h"
#include "core/diffusion/runga_kutta_grid.h"
#include "core/diffusion/stencil_grid.h"

namespace bdm {

void ModelInitializer::DefineSubstance(size_t substance_id,
                                       const std::string& substance_name,
                                       double diffusion_coeff,
                                       double decay_constant, int resolution) {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  auto* rm = sim->GetResourceManager();
  DiffusionGrid* dgrid = nullptr;
  if (param->diffusion_method == "euler") {
    dgrid = new EulerGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution);
  } else if (param->diffusion_method == "stencil") {
    dgrid = new StencilGrid(substance_id, substance_name, diffusion_coeff,
                            decay_constant, resolution);
  } else if (param->diffusion_method == "runga-kutta") {
    dgrid = new RungaKuttaGrid(substance_id, substance_name, diffusion_coeff,
                               decay_constant, resolution);
  } else {
    Log::Error("ModelInitializer::DefineSubstance", "Diffusion method '",
               param->diffusion_method,
               "' does not exist. Defaulting to 'euler'");
    dgrid = new EulerGrid(substance_id, substance_name, diffusion_coeff,
                          decay_constant, resolution);
  }

  rm->AddDiffusionGrid(dgrid);
}

}  // namespace bdm
