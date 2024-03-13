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
  EulerDepletionGrid(int substance_id, std::string substance_name, real_t dc,
                     real_t mu, int resolution = 10,
                     std::vector<real_t> binding_coefficients = {},
                     std::vector<int> binding_substances = {})
      : EulerGrid(substance_id, std::move(substance_name), dc, mu, resolution),
        binding_coefficients_(std::move(binding_coefficients)),
        binding_substances_(std::move(binding_substances)) {}

  /// Simulates the Diffusion with EulerGrid::DiffuseWithClosedEdge and
  /// subsequently depletes the substance according to binding_substances_ and
  /// binding_coefficients_. See ApplyDepletion for details.
  void DiffuseWithClosedEdge(real_t dt) override;
  /// Simulates the Diffusion with EulerGrid::DiffuseWithOpenEdge and
  /// subsequently depletes the substance according to binding_substances_ and
  /// binding_coefficients_. See ApplyDepletion for details.
  void DiffuseWithOpenEdge(real_t dt) override;
  /// Simulates the Diffusion with EulerGrid::DiffuseWithDirichlet and
  /// subsequently depletes the substance according to binding_substances_ and
  /// binding_coefficients_. See ApplyDepletion for details.
  void DiffuseWithDirichlet(real_t dt) override;
  /// Simulates the Diffusion with EulerGrid::DiffuseWithNeumann and
  /// subsequently depletes the substance according to binding_substances_ and
  /// binding_coefficients_. See ApplyDepletion for details.
  void DiffuseWithNeumann(real_t dt) override;
  /// Simulates the Diffusion with EulerGrid::DiffuseWithPeriodic and
  /// subsequently depletes the substance according to binding_substances_ and
  /// binding_coefficients_. See ApplyDepletion for details.
  void DiffuseWithPeriodic(real_t dt) override;

  // To avoid missing substances or coefficients, name of the sub and binding
  // coefficient must be set at the same time

  /// @brief Sets a binding substance and coefficient for the substance
  /// @param bnd_sub ID of the binding substance
  /// @param bnd_coeff Binding coefficient that determines the rate of
  /// depletion
  void SetBindingSubstance(int bnd_sub, real_t bnd_coeff) {
    binding_substances_.push_back(bnd_sub);
    binding_coefficients_.push_back(bnd_coeff);
  }

  /// Get the vector of binding substances (vector of IDs)
  std::vector<int> GetBindingSubstances() const { return binding_substances_; }
  /// Get the vector of binding coefficients (vector of coefficients)
  std::vector<real_t> GetBindingCoefficients() const {
    return binding_coefficients_;
  }

 private:
  /// @brief Depletes the substance according to binding_substances_ and
  /// binding_coefficients_. Iterates over all grid points and calculates the
  /// new concentration according to the depletion. See class description.
  /// @param dt Time step
  void ApplyDepletion(real_t dt);

  /// Vector of binding coefficients for each binding substance. All
  /// coefficients should be positive.
  std::vector<real_t> binding_coefficients_ = {};
  /// Vector of binding substances.
  std::vector<int> binding_substances_ = {};

  BDM_CLASS_DEF_OVERRIDE(EulerDepletionGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_EULER_DEPLETION_GRID_H_
