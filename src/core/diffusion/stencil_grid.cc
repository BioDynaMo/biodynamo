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

#include "core/diffusion/stencil_grid.h"

namespace bdm {

void StencilGrid::DiffuseWithClosedEdge() {
  auto nx = resolution_;
  auto ny = resolution_;
  auto nz = resolution_;

#define YBF 16
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x;
        int c, n, s, b, t;
        x = 0;
        c = x + y * nx + z * nx * ny;
        n = (y == 0) ? c : c - nx;
        s = (y == ny - 1) ? c : c + nx;
        b = (z == 0) ? c : c - nx * ny;
        t = (z == nz - 1) ? c : c + nx * ny;
        c2_[c] = (dc_[0] * c1_[c] + dc_[1] * c1_[c] + dc_[2] * c1_[c + 1] +
                  dc_[3] * c1_[s] + dc_[4] * c1_[n] + dc_[5] * c1_[b] +
                  dc_[6] * c1_[t]) *
                 (1 - mu_);
#pragma omp simd
        for (x = 1; x < nx - 1; x++) {
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2_[c] = (dc_[0] * c1_[c] + dc_[1] * c1_[c - 1] +
                    dc_[2] * c1_[c + 1] + dc_[3] * c1_[s] + dc_[4] * c1_[n] +
                    dc_[5] * c1_[b] + dc_[6] * c1_[t]) *
                   (1 - mu_);
        }
        ++c;
        ++n;
        ++s;
        ++b;
        ++t;
        c2_[c] = (dc_[0] * c1_[c] + dc_[1] * c1_[c - 1] + dc_[2] * c1_[c] +
                  dc_[3] * c1_[s] + dc_[4] * c1_[n] + dc_[5] * c1_[b] +
                  dc_[6] * c1_[t]) *
                 (1 - mu_);
      }  // tile ny
    }    // tile nz
  }      // block ny
  c1_.swap(c2_);
}

void StencilGrid::DiffuseWithOpenEdge() {
  int nx = resolution_;
  int ny = resolution_;
  int nz = resolution_;

#define YBF 16
#pragma omp parallel for collapse(2)
  for (int yy = 0; yy < ny; yy += YBF) {
    for (int z = 0; z < nz; z++) {
      // To let the edges bleed we set some diffusion coefficients
      // to zero. This prevents substance building up at the edges
      auto dc_2_ = dc_;
      int ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (int y = yy; y < ymax; y++) {
        dc_2_ = dc_;
        int x;
        int c, n, s, b, t;
        x = 0;
        c = x + y * nx + z * nx * ny;
        if (y == 0) {
          n = c;
          dc_2_[4] = 0;
        } else {
          n = c - nx;
        }
        if (y == (ny - 1)) {
          s = c;
          dc_2_[3] = 0;
        } else {
          s = c + nx;
        }
        if (z == 0) {
          b = c;
          dc_2_[5] = 0;
        } else {
          b = c - nx * ny;
        }
        if (z == (nz - 1)) {
          t = c;
          dc_2_[6] = 0;
        } else {
          t = c + nx * ny;
        }
        // x = 0; we leak out substances past this edge (so multiply by 0)
        c2_[c] = (dc_2_[0] * c1_[c] + 0 * c1_[c] + dc_2_[2] * c1_[c + 1] +
                  dc_2_[3] * c1_[s] + dc_2_[4] * c1_[n] + dc_2_[5] * c1_[b] +
                  dc_2_[6] * c1_[t]) *
                 (1 - mu_);
#pragma omp simd
        for (x = 1; x < nx - 1; x++) {
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2_[c] = (dc_2_[0] * c1_[c] + dc_2_[1] * c1_[c - 1] +
                    dc_2_[2] * c1_[c + 1] + dc_2_[3] * c1_[s] +
                    dc_2_[4] * c1_[n] + dc_2_[5] * c1_[b] + dc_2_[6] * c1_[t]) *
                   (1 - mu_);
        }
        ++c;
        ++n;
        ++s;
        ++b;
        ++t;
        // x = nx-1; we leak out substances past this edge (so multiply by 0)
        c2_[c] = (dc_2_[0] * c1_[c] + dc_2_[1] * c1_[c - 1] + 0 * c1_[c] +
                  dc_2_[3] * c1_[s] + dc_2_[4] * c1_[n] + dc_2_[5] * c1_[b] +
                  dc_2_[6] * c1_[t]) *
                 (1 - mu_);
      }  // tile ny
    }    // tile nz
  }      // block ny
  c1_.swap(c2_);
}

}  // namespace bdm
