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

namespace bdm {

void RungeKuttaGrid::DiffuseWithClosedEdge(double dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const double ibl2 = 1 / (box_length_ * box_length_);
  const double d = 1 - dc_[0];
  double step = diffusion_step_;
  double h = dt / step;
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

              double h2 = h / 2.0;

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

void RungeKuttaGrid::DiffuseWithOpenEdge(double dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const double ibl2 = 1 / (box_length_ * box_length_);
  std::array<int, 4> l;
  const double d = 1 - dc_[0];

#define YBF 16
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

        l.fill(1);

        if (y == 0) {
          n = c;
          l[0] = 0;
        } else {
          n = c - nx;
        }

        if (y == ny - 1) {
          s = c;
          l[1] = 0;
        } else {
          s = c + nx;
        }

        if (z == 0) {
          b = c;
          l[2] = 0;
        } else {
          b = c - nx * ny;
        }

        if (z == nz - 1) {
          t = c;
          l[3] = 0;
        } else {
          t = c + nx * ny;
        }

        c2_[c] = (c1_[c] + d * dt * (0 - 2 * c1_[c] + c1_[c + 1]) * ibl2 +
                  d * dt * (c1_[s] - 2 * c1_[c] + c1_[n]) * ibl2 +
                  d * dt * (c1_[b] - 2 * c1_[c] + c1_[t]) * ibl2) *
                 (1 - mu_);
#pragma omp simd
        for (x = 1; x < nx - 1; x++) {
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2_[c] =
              (c1_[c] + d * dt * (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1]) * ibl2 +
               d * dt * (l[0] * c1_[s] - 2 * c1_[c] + l[1] * c1_[n]) * ibl2 +
               d * dt * (l[2] * c1_[b] - 2 * c1_[c] + l[3] * c1_[t]) * ibl2) *
              (1 - mu_);
        }
        ++c;
        ++n;
        ++s;
        ++b;
        ++t;
        c2_[c] = (c1_[c] + d * dt * (c1_[c - 1] - 2 * c1_[c] + 0) * ibl2 +
                  d * dt * (c1_[s] - 2 * c1_[c] + c1_[n]) * ibl2 +
                  d * dt * (c1_[b] - 2 * c1_[c] + c1_[t]) * ibl2) *
                 (1 - mu_);
      }  // tile ny
    }    // tile nz
  }      // block ny
  c1_.swap(c2_);
}

}  // namespace bdm
