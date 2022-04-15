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
   and substance depletion \f$ \partial_t u = \nabla D \nabla u - \mu u - \mu'
   v u \f$.

For a single depleting substance with concentration c' and binding coefficient
mu_' the FCTS method leads to the following relation: c2_[c] = c1_[c] * (1 -
(mu_ + c'1_[c] * mu_') * dt) + (d * dt * ibl2) * (c1_[c - 1] - 2 * c1_[c] +
c1_[c + 1] + c1_[s] - 2 * c1_[c] + c1_[n] + c1_[b] - 2 * c1_[c] + c1_[t])

*/
class EulerDepletionGrid : public EulerGrid {
 public:
  EulerDepletionGrid() = default;
  EulerDepletionGrid(size_t substance_id, std::string substance_name, double dc,
                     double mu, int resolution = 10,
                     std::vector<double> binding_coefficients = {},
                     std::vector<size_t> binding_substances = {})
      : EulerGrid(substance_id, std::move(substance_name), dc, mu, resolution),
        binding_coefficients_(std::move(binding_coefficients)),
        binding_substances_(std::move(binding_substances)) {}

  void DiffuseWithClosedEdge(double dt) override;
  void DiffuseWithOpenEdge(double dt) override;
  void DiffuseWithDirichlet(double dt) override;
  void DiffuseWithNeumann(double dt) override;

  // To avoid missing substances or coefficients, name of the sub and binding
  // coefficient must be set at the same time
  void SetBindingSubstance(size_t bnd_sub, double bnd_coeff) {
    binding_substances_.push_back(bnd_sub);
    binding_coefficients_.push_back(bnd_coeff);
  }

  std::vector<size_t> GetBindingSubstances() const {
    return binding_substances_;
  }
  std::vector<double> GetBindingCoefficients() const {
    return binding_coefficients_;
  }

 private:
  void ApplyDepletion(double dt);

  std::vector<double> binding_coefficients_ = {};
  std::vector<size_t> binding_substances_ = {};

  BDM_CLASS_DEF_OVERRIDE(EulerDepletionGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_EULER_DEPLETION_GRID_H_
