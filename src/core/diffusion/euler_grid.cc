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

// Function to move <val> into the range [<min>, <max>]. If it is in the range
// already, it is returned unchanged. Else, the closest boundary is returned.
inline size_t Clamp(size_t val, size_t min, size_t max) {
  if (val < min) {
    return min;
  }
  if (val > max) {
    return max;
  }
  return val;
}

void EulerGrid::DiffuseWithClosedEdge(real_t dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const real_t ibl2 = 1 / (box_length_ * box_length_);
  const real_t d = 1 - dc_[0];

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
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

void EulerGrid::DiffuseWithOpenEdge(real_t dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const real_t ibl2 = 1 / (box_length_ * box_length_);
  const real_t d = 1 - dc_[0];
  std::array<int, 4> l;

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
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

void EulerGrid::DiffuseWithDirichlet(real_t dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;

  const real_t ibl2 = 1 / (box_length_ * box_length_);
  const real_t d = 1 - dc_[0];

  const auto sim_time = GetSimulatedTime();

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        c = x + y * nx + z * nx * ny;
#pragma omp simd
        for (x = 0; x < nx; x++) {
          if (x == 0 || x == (nx - 1) || y == 0 || y == (ny - 1) || z == 0 ||
              z == (nz - 1)) {
            // For all boxes on the boundary, we simply evaluate the boundary
            real_t real_x = grid_dimensions_[0] + x * box_length_;
            real_t real_y = grid_dimensions_[0] + y * box_length_;
            real_t real_z = grid_dimensions_[0] + z * box_length_;
            c2_[c] =
                boundary_condition_->Evaluate(real_x, real_y, real_z, sim_time);
          } else {
            // For inner boxes, we compute the regular stencil update
            n = c - nx;
            s = c + nx;
            b = c - nx * ny;
            t = c + nx * ny;

            c2_[c] = c1_[c] * (1 - mu_ * dt) +
                     (d * dt * ibl2) *
                         (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1] + c1_[s] -
                          2 * c1_[c] + c1_[n] + c1_[b] - 2 * c1_[c] + c1_[t]);
          }
          ++c;
        }
      }  // tile ny
    }    // tile nz
  }      // block ny
  c1_.swap(c2_);
}

void EulerGrid::DiffuseWithNeumann(real_t dt) {
  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;
  const auto num_boxes = nx * ny * nz;

  const real_t ibl2 = 1 / (box_length_ * box_length_);
  const real_t d = 1 - dc_[0];

  const auto sim_time = GetSimulatedTime();

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        c = x + y * nx + z * nx * ny;
#pragma omp simd
        for (x = 0; x < nx; x++) {
          n = c - nx;
          s = c + nx;
          b = c - nx * ny;
          t = c + nx * ny;

          // Clamp to avoid out of bounds access. Clamped values are initialized
          // to a wrong value but will be overwritten by the boundary condition
          // evaluation. All other values are correct.
          real_t left{c1_[Clamp(c - 1, 0, num_boxes - 1)]};
          real_t right{c1_[Clamp(c + 1, 0, num_boxes - 1)]};
          real_t bottom{c1_[Clamp(b, 0, num_boxes - 1)]};
          real_t top{c1_[Clamp(t, 0, num_boxes - 1)]};
          real_t north{c1_[Clamp(n, 0, num_boxes - 1)]};
          real_t south{c1_[Clamp(s, 0, num_boxes - 1)]};
          real_t center_factor{6.0};

          if (x == 0 || x == (nx - 1) || y == 0 || y == (ny - 1) || z == 0 ||
              z == (nz - 1)) {
            real_t real_x = grid_dimensions_[0] + x * box_length_;
            real_t real_y = grid_dimensions_[0] + y * box_length_;
            real_t real_z = grid_dimensions_[0] + z * box_length_;
            real_t boundary_value =
                -box_length_ *
                boundary_condition_->Evaluate(real_x, real_y, real_z, sim_time);

            if (x == 0) {
              left = boundary_value;
              center_factor -= 1.0;
            } else if (x == (nx - 1)) {
              right = boundary_value;
              center_factor -= 1.0;
            }

            if (y == 0) {
              north = boundary_value;
              center_factor -= 1.0;
            } else if (y == (ny - 1)) {
              south = boundary_value;
              center_factor -= 1.0;
            }

            if (z == 0) {
              bottom = boundary_value;
              center_factor -= 1.0;
            } else if (z == (nz - 1)) {
              top = boundary_value;
              center_factor -= 1.0;
            }
          }

          c2_[c] = c1_[c] * (1 - mu_ * dt) +
                   (d * dt * ibl2) * (left + right + south + north + top +
                                      bottom - center_factor * c1_[c]);

          ++c;
        }
      }  // tile ny
    }    // tile nz
  }      // block ny
  c1_.swap(c2_);
}

void EulerGrid::DiffuseWithPeriodic(real_t dt) {

  const auto nx = resolution_;
  const auto ny = resolution_;
  const auto nz = resolution_;
  const auto num_boxes = nx * ny * nz;

  const real_t dx = box_length_;
  const real_t d = 1 - dc_[0];

  constexpr size_t YBF = 16;
#pragma omp parallel for collapse(2)
  for (size_t yy = 0; yy < ny; yy += YBF) {
    for (size_t z = 0; z < nz; z++) {
      size_t ymax = yy + YBF;
      if (ymax >= ny) {
        ymax = ny;
      }
      for (size_t y = yy; y < ymax; y++) {
        size_t x{0};
        size_t c{0};
        size_t l{0};
        size_t r{0};
        size_t n{0};
        size_t s{0};
        size_t b{0};
        size_t t{0};
        c = x + y * nx + z * nx * ny;
#pragma omp simd
        for (x = 0; x < nx; x++) {
          l = c - 1;
          r = c + 1;
          n = c - nx;
          s = c + nx;
          b = c - nx * ny;
          t = c + nx * ny;

          // Clamp to avoid out of bounds access. Clamped values are initialized
          // to a wrong value but will be overwritten by the boundary condition
          // evaluation. All other values are correct.
          real_t left{c1_[Clamp(l, 0, num_boxes - 1)]};
          real_t right{c1_[Clamp(r, 0, num_boxes - 1)]};
          real_t bottom{c1_[Clamp(b, 0, num_boxes - 1)]};
          real_t top{c1_[Clamp(t, 0, num_boxes - 1)]};
          real_t north{c1_[Clamp(n, 0, num_boxes - 1)]};
          real_t south{c1_[Clamp(s, 0, num_boxes - 1)]};

          if (x == 0 || x == (nx - 1) || y == 0 || y == (ny - 1) || z == 0 ||
              z == (nz - 1)) {

            // For each box on the boundary, we find the concentration of the 
            // box on the opposite side of the simulation:
            //    |                          |
            //    |u0  u1  u2 .... un-2  un-1|
            //    |                          |
            // The diffusion in u0 will be determined by the concentration in 
            // u1 and un-1.


            if (x == 0) {
              l = nx-1 + y * nx + z * nx * ny;
              left = c1_[l];
            } else if (x == (nx - 1)) {
              r = 0 + y * nx + z * nx * ny;
              right = c1_[r];
            }

            if (y == 0) {
              n = x + (ny-1) * nx + z * nx * ny;
              north = c1_[n];
            } else if (y == (ny - 1)) {
              s = x + 0 * nx + z * nx * ny;
              south = c1_[s];
            }

            if (z == 0) {
              b = x + y * nx + (nz-1) * nx * ny;
              bottom = c1_[b];
            } else if (z == (nz - 1)) {
              t = x + y * nx + 0 * nx * ny;
              top = c1_[t];
            }
          }

          c2_[c] = c1_[c] * (1 - (mu_ * dt))
                            + ( (d * dt / (dx*dx) ) * (left + right + north + south
                                                       + top + bottom - 6.0*c1_[c]) );

          ++c;
        }
      }  // tile ny
    }    // tile nz
  }      // block ny
  c1_.swap(c2_);
}

}  // namespace bdm
