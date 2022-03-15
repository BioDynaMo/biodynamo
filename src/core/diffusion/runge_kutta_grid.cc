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

#include "core/diffusion/runge_kutta_grid.h"
#include "core/util/log.h"

namespace bdm {

void RungeKuttaGrid::DiffuseWithClosedEdge(real dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const real ibl2 = 1 / (box_length_ * box_length_);
  const real d = 1 - dc_[0];
  real step = diffusion_step_;
  real h = dt / step;
#define YBF 16
  for (size_t i = 0; i < step; i++) {
    for (size_t order = 0; order < 2; order++) {
#pragma omp parallel for collapse(2)
      for (size_t yy = 0; yy < ny; yy += YBF) {
        for (size_t z = 0; z < nz; z++) {
          size_t ymax = yy + YBF;
          if (ymax >= ny) {
            ymax = ny;
          }
          for (size_t y = yy; y < ymax; y++) {
            size_t x = 0;
            int c, n, s, b, t;
            c = x + y * nx + z * nx * ny;
#pragma omp simd
            for (x = 1; x < nx - 1; x++) {
              ++c;

              if (y == 0 || y == (ny - 1) || z == 0 || z == (nz - 1)) {
                continue;
              }

              n = c - nx;
              s = c + nx;
              b = c - nx * ny;
              t = c + nx * ny;

              real h2 = h / 2.0;

              if (order == 0) {
                k_[0] = (d * (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1]) * ibl2 +
                         d * (c1_[s] - 2 * c1_[c] + c1_[n]) * ibl2 +
                         d * (c1_[b] - 2 * c1_[c] + c1_[t]) * ibl2);
                r1_[c] = c1_[c] + (k_[0] * h2);
              } else if (order == 1) {
                k_[1] = (d * (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1]) * ibl2 +
                         d * (c1_[s] - 2 * c1_[c] + c1_[n]) * ibl2 +
                         d * (c1_[b] - 2 * c1_[c] + c1_[t]) * ibl2);

                c2_[c] = c1_[c] + (k_[1] * h);
              }
            }
          }  // tile ny
        }    // tile nz
      }      // block ny
    }
    c1_.swap(c2_);
  }
}

void RungeKuttaGrid::DiffuseWithOpenEdge(real dt) {
  Log::Fatal(
      "RungeKuttaGrid::DiffuseWithOpenEdge",
      "Open Edge Diffusion is not implemented, please use the EulerGrid.");
}

}  // namespace bdm
