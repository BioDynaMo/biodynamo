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

#include "core/diffusion/euler_grid.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

namespace bdm {

void EulerGrid::DiffuseWithClosedEdge(double dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const double ibl2 = 1 / (box_length_ * box_length_);
  const double d = 1 - dc_[0];

  if (isDepleted_ == true) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    auto* depletesDg = rm->GetDiffusionGrid(isDepletedBy_);
    auto* allConcsDepletes = depletesDg->GetAllConcentrations();

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

            if (z == 1) {
              c1_[b] = c1_[c];
            }

            if (z == (nz - 2)) {
              c1_[t] = c1_[c];
            }

            if (y == 1) {
              c1_[n] = c1_[c];
            }

            if (y == (ny - 2)) {
              c1_[s] = c1_[c];
            }

            if (x == 1) {
              c1_[c - 1] = c1_[c];
            }

            if (x == (nx - 2)) {
              c1_[c + 1] = c1_[c];
            }

            c2_[c] =
                c1_[c] *
                    (1 -
                     (mu_ + binding_coefficient_ * allConcsDepletes[c]) * dt) +
                (d * dt * ibl2) *
                    (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1] + c1_[s] -
                     2 * c1_[c] + c1_[n] + c1_[b] - 2 * c1_[c] + c1_[t]);
          }
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  } else {
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

            c2_[c] = c1_[c] * (1 - mu_ * dt) +
                     (d * dt * ibl2) *
                         (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1] + c1_[s] -
                          2 * c1_[c] + c1_[n] + c1_[b] - 2 * c1_[c] + c1_[t]);
          }
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  }
}

void EulerGrid::DiffuseWithOpenEdge(double dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const double ibl2 = 1 / (box_length_ * box_length_);
  const double d = 1 - dc_[0];
  std::array<int, 4> l;

  if (isDepleted_ == true) {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();

    auto* depletesDg = rm->GetDiffusionGrid(isDepletedBy_);
    auto* allConcsDepletes = depletesDg->GetAllConcentrations();

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

          c2_[c] =
              c1_[c] * (1 - (mu_ + binding_coefficient_ * allConcsDepletes[c]) *
                                dt) +
              (d * dt * ibl2) *
                  (0 - 2 * c1_[c] + c1_[c + 1] + c1_[s] - 2 * c1_[c] + c1_[n] +
                   c1_[b] - 2 * c1_[c] + c1_[t]);
#pragma omp simd
          for (x = 1; x < nx - 1; x++) {
            ++c;
            ++n;
            ++s;
            ++b;
            ++t;
            c2_[c] =
                c1_[c] *
                    (1 -
                     (mu_ + binding_coefficient_ * allConcsDepletes[c]) * dt) +
                (d * dt * ibl2) * (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1] +
                                   l[0] * c1_[s] - 2 * c1_[c] + l[1] * c1_[n] +
                                   l[2] * c1_[b] - 2 * c1_[c] + l[3] * c1_[t]);
          }
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2_[c] =
              c1_[c] * (1 - (mu_ + binding_coefficient_ * allConcsDepletes[c]) *
                                dt) +
              (d * dt * ibl2) *
                  (c1_[c - 1] - 2 * c1_[c] + 0 + c1_[s] - 2 * c1_[c] + c1_[n] +
                   c1_[b] - 2 * c1_[c] + c1_[t]);
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  } else {
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

          c2_[c] = c1_[c] * (1 - mu_ * dt) +
                   (d * dt * ibl2) *
                       (0 - 2 * c1_[c] + c1_[c + 1] + c1_[s] - 2 * c1_[c] +
                        c1_[n] + c1_[b] - 2 * c1_[c] + c1_[t]);
#pragma omp simd
          for (x = 1; x < nx - 1; x++) {
            ++c;
            ++n;
            ++s;
            ++b;
            ++t;
            c2_[c] =
                c1_[c] * (1 - mu_ * dt) +
                (d * dt * ibl2) * (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1] +
                                   l[0] * c1_[s] - 2 * c1_[c] + l[1] * c1_[n] +
                                   l[2] * c1_[b] - 2 * c1_[c] + l[3] * c1_[t]);
          }
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2_[c] = c1_[c] * (1 - mu_ * dt) +
                   (d * dt * ibl2) *
                       (c1_[c - 1] - 2 * c1_[c] + 0 + c1_[s] - 2 * c1_[c] +
                        c1_[n] + c1_[b] - 2 * c1_[c] + c1_[t]);
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  }
}

}  // namespace bdm
