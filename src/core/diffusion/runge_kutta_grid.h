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

#ifndef CORE_DIFFUSION_RUNGE_KUTTA_GRID_H_
#define CORE_DIFFUSION_RUNGE_KUTTA_GRID_H_

#include <utility>

#include "core/diffusion/diffusion_grid.h"
#include "core/util/root.h"

namespace bdm {

/** @brief Continuum model for the 3D heat equation
           \f$ \partial_t u = \nabla D \nabla u \f$.

  This alternative solution to the finite difference method is based on an
  explicit second order Runge-Kutta scheme.  Taking a approach which is forward
  in time and central in space. Due to the more sophisticated time integration,
  the solution converges better than the EulerGrid. However, this causes a
  tradeoff leading to increased computational effort.
*/
class RungeKuttaGrid : public DiffusionGrid {
 public:
  RungeKuttaGrid() = default;
  RungeKuttaGrid(int substance_id, std::string substance_name, double dc,
                 int resolution = 10)
      : DiffusionGrid(substance_id, std::move(substance_name), dc, 0.0,
                      resolution) {}

  void Initialize() override {
    DiffusionGrid::Initialize();
    // If we are utilising the Runge-Kutta method we need to resize an
    // additional vector, this will be used in estimating the concentration
    // between diffusion steps.
    r1_.resize(total_num_boxes_);
  }

  void Update() override {
    DiffusionGrid::Update();
    r1_.resize(total_num_boxes_);
  }

  void DiffuseWithClosedEdge(double dt) override;
  void DiffuseWithOpenEdge(double dt) override;

 private:
  /// Buffers for Runge Kutta
  ParallelResizeVector<double> r1_ = {};
  /// k array for runge-kutta.
  std::array<double, 2> k_ = {};
  /// Number of steps for RK diffusion grid;
  unsigned int diffusion_step_ = 1;
  BDM_CLASS_DEF_OVERRIDE(RungeKuttaGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_RUNGE_KUTTA_GRID_H_
