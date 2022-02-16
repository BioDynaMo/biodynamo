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

#ifndef CORE_DIFFUSION_EULER_GRID_H_
#define CORE_DIFFUSION_EULER_GRID_H_

#include <utility>

#include "core/diffusion/diffusion_grid.h"

namespace bdm {

/** @brief Continuum model for the 3D heat equation with exponential decay
           \f$ \partial_t u = \nabla D \nabla u - \mu u \f$.

  The solution is based on the finite difference method, more precisely, a
  forward difference in time and a central difference in space. Thus, the
  employed scheme is referred to as FTCS scheme. Time time integration is
  explicit resulting in a stability condition
  \f$ \frac{D \Delta t}{\Delta x^2} < \frac{1}{6} \f$. The error of the scheme
  scales linearly with time (forward difference of first order) and quadratic
  with the spatial discretization (central difference of second order). The
  time integration corresponds to the forward Euler, thus the name of the class.

  Further infomation:
    - <a href="https://biodynamo.org/docs/userguide/diffusion/">
      BioDynaMo User Guide </a>
    - <a
      href="http://dma.dima.uniroma1.it/users/lsa_adn/MATERIALE/FDheat.pdf">
      Recktenwald, Finite-Difference Approximations to the Heat Equation,
      2004</a>
*/
class EulerGrid : public DiffusionGrid {
 public:
  EulerGrid() {}
  EulerGrid(int substance_id, std::string substance_name, double dc, double mu,
            int resolution = 11)
      : DiffusionGrid(substance_id, std::move(substance_name), dc, mu,
                      resolution) {}

  void DiffuseWithClosedEdge(double dt) override;

  void DiffuseWithOpenEdge(double dt) override;

 private:
  BDM_CLASS_DEF_OVERRIDE(EulerGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_EULER_GRID_H_
