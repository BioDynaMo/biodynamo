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

#ifndef CORE_DIFFUSION_RUNGA_KUTTA_GRID_H_
#define CORE_DIFFUSION_RUNGA_KUTTA_GRID_H_

#include <utility>

#include "core/diffusion/diffusion_grid.h"
#include "core/util/root.h"

namespace bdm {

class RungaKuttaGrid : public DiffusionGrid {
 public:
  RungaKuttaGrid() {}
  RungaKuttaGrid(int substance_id, std::string substance_name, double dc,
                 double mu, int resolution = 11)
      : DiffusionGrid(substance_id, std::move(substance_name), dc, mu,
                      resolution) {}

  void Initialize() override {
    DiffusionGrid::Initialize();
    // If we are utilising the Runge-Kutta method we need to resize an
    // additional vector, this will be used in estimating the concentration
    // between diffsuion steps.
    r1_.resize(total_num_boxes_);
  }

  void Update() override {
    DiffusionGrid::Update();
    r1_.resize(total_num_boxes_);
  }

  void DiffuseWithClosedEdge() override;
  void DiffuseWithOpenEdge() override;

 private:
  /// Buffers for Runge Kutta
  ParallelResizeVector<double> r1_ = {};
  /// k array for runge-kutta.
  std::array<double, 2> k_ = {};
  /// Number of steps for RK diffusion grid;
  unsigned int diffusion_step_ = 1;
  BDM_CLASS_DEF_OVERRIDE(RungaKuttaGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_RUNGA_KUTTA_GRID_H_
