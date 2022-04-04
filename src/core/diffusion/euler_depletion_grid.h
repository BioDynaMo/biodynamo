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

#ifndef CORE_DIFFUSION_EULER_DEPLETION_GRID_H_
#define CORE_DIFFUSION_EULER_DEPLETION_GRID_H_

#include <utility>
#include "core/diffusion/diffusion_grid.h"
#include "core/diffusion/euler_grid.h"

namespace bdm {

/** @brief Continuum model for the 3D diffusion equation with exponential decay
   and substance depletion \f$ \partial_t u = \nabla D \nabla u - \mu u - \delta
   v \f$.
*/
class EulerDepletionGrid : public EulerGrid {
 public:
  EulerDepletionGrid() = default;
  EulerDepletionGrid(int substance_id, std::string substance_name, double dc,
                     double mu, int resolution = 10,
                     std::vector<double> binding_coefficients = {},
                     std::vector<size_t> binding_substances = {})
      : EulerGrid(substance_id, std::move(substance_name), dc, mu, resolution),
        binding_coefficients_(std::move(binding_coefficients)),
        binding_substances_(std::move(binding_substances)) {}

  void DiffuseWithClosedEdge(double dt) override;

  void DiffuseWithOpenEdge(double dt) override;

 private:
  /// Depletion
  std::vector<double> binding_coefficients_ = {};
  std::vector<size_t> binding_substances_ = {};

  BDM_CLASS_DEF_OVERRIDE(EulerDepletionGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_EULER_DEPLETION_GRID_H_
