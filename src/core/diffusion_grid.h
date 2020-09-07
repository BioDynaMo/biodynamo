// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_DIFFUSION_GRID_H_
#define CORE_DIFFUSION_GRID_H_

#include <assert.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include "core/util/root.h"

#include "core/container/math_array.h"
#include "core/container/parallel_resize_vector.h"
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/math.h"

namespace bdm {

/// A class that computes the diffusion of extracellular substances
/// It maintains the concentration and gradient of a single substance
class DiffusionGrid {
 public:
  explicit DiffusionGrid(TRootIOCtor* p) {}
  DiffusionGrid(int substance_id, std::string substance_name, double dc,
                double mu, int resolution = 11, unsigned int diffusion_step = 1)
      : substance_(substance_id),
        substance_name_(substance_name),
        dc_({{1 - dc, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6}}),
        mu_(mu),
        resolution_(resolution),
        diffusion_step_(diffusion_step) {}

  virtual ~DiffusionGrid() {}

  /// @brief      Initializes the grid by calculating the grid dimensions
  ///             and number of boxes along the axis from the input arguments
  ///
  /// @param[in]  grid_dimensions  The grid dimensions
  /// @param[in]  box_length       The box length
  ///
  void Initialize(const std::array<int32_t, 6>& grid_dimensions) {
    // Get grid properties from neighbor grid
    grid_dimensions_ = grid_dimensions;
    assert(resolution_ > 0 && "The resolution cannot be zero!");

    num_boxes_axis_[0] = resolution_;
    num_boxes_axis_[1] = resolution_;
    num_boxes_axis_[2] = resolution_;

    // Example: diffusion grid dimensions from 0-40 and resolution
    // of 4. Resolution must be adjusted otherwise one data pointer will be
    // missing.
    // Without adjustment:
    //   box_length_: 10
    //   data points {0, 10, 20, 30} - 40 will be misssing!
    // With adjustment
    //   box_length_: 13.3
    //   data points: {0, 13.3, 26.6, 39.9}
    box_length_ = (grid_dimensions_[1] - grid_dimensions_[0]) /
                  static_cast<double>(resolution_ - 1);
    ParametersCheck();

    box_volume_ = box_length_ * box_length_ * box_length_;

    assert(box_length_ > 0 &&
           "Box length of diffusion grid must be greater than zero!");

    // Set the parity of the number of boxes along the dimensions (since all
    // dimensions are the same, we just take the x-axis here)
    parity_ = num_boxes_axis_[0] % 2;

    total_num_boxes_ =
        num_boxes_axis_[0] * num_boxes_axis_[1] * num_boxes_axis_[2];

    // Allocate memory for the concentration and gradient arrays
    c1_.resize(total_num_boxes_);
    c2_.resize(total_num_boxes_);
    gradients_.resize(3 * total_num_boxes_);

    // If we are utilising the Runge-Kutta method we need to resize an
    // additional vector, this will be used in estimating the concentration
    // between diffsuion steps.
    auto* param = Simulation::GetActive()->GetParam();
    if (param->diffusion_type_ == "RK") {
      r1_.resize(total_num_boxes_);
    }

    initialized_ = true;
  }

  void ParametersCheck() {
    if (((1 - dc_[0]) * dt_) / (box_length_ * box_length_) >= (1.0 / 6)) {
      Log::Fatal(
          "DiffusionGrid",
          "The specified parameters of the diffusion grid with substance [",
          substance_name_,
          "] will result in unphysical behavior (diffusion coefficient = ",
          (1 - dc_[0]), ", resolution = ", resolution_,
          "). Please refer to the user guide for more information.");
    } else if (diffusion_step_ == 0) {
      Log::Fatal("DiffusionGrid",
                 " The specified amount of diffusion steps for the grid with "
                 "substance [",
                 substance_name_,
                 "] is not greater than or equal to 1, "
                 "correct this and run the simulation again.");
    }
  }

  void RunInitializers() {
    assert(num_boxes_axis_[0] > 0 &&
           "The number of boxes along an axis was found to be zero!");
    if (initializers_.empty()) {
      return;
    }

    auto nx = num_boxes_axis_[0];
    auto ny = num_boxes_axis_[1];
    auto nz = num_boxes_axis_[2];

    // Apply all functions that initialize this diffusion grid
    for (size_t f = 0; f < initializers_.size(); f++) {
      for (size_t x = 0; x < nx; x++) {
        double real_x = grid_dimensions_[0] + x * box_length_;
        for (size_t y = 0; y < ny; y++) {
          double real_y = grid_dimensions_[2] + y * box_length_;
          for (size_t z = 0; z < nz; z++) {
            double real_z = grid_dimensions_[4] + z * box_length_;
            std::array<uint32_t, 3> box_coord;
            box_coord[0] = x;
            box_coord[1] = y;
            box_coord[2] = z;
            size_t idx = GetBoxIndex(box_coord);
            IncreaseConcentrationBy(idx,
                                    initializers_[f](real_x, real_y, real_z));
          }
        }
      }
    }

    // Clear the initializer to free up space
    initializers_.clear();
    initializers_.shrink_to_fit();
  }

  /// @brief      Updates the grid dimensions, based on the given threshold
  ///             values. The diffusion grid dimensions need always be larger
  ///             than the neighbor grid dimensions, so that each simulation
  ///             object can obtain its local concentration / gradient
  ///
  /// @param[in]  threshold_dimensions  The threshold values
  ///
  void Update(const std::array<int32_t, 2>& threshold_dimensions) {
    // Update the grid dimensions such that each dimension ranges from
    // {treshold_dimensions[0] - treshold_dimensions[1]}
    auto min_gd = threshold_dimensions[0];
    auto max_gd = threshold_dimensions[1];
    grid_dimensions_ = {min_gd, max_gd, min_gd, max_gd, min_gd, max_gd};

    // If the grid is not perfectly divisible along each dimension by the
    // box length, extend the grid so that it is
    int dimension_length = max_gd - min_gd;
    for (int i = 0; i < 3; i++) {
      int r = fmod(dimension_length, box_length_);
      if (r > 1e-9) {
        // std::abs for the case that box_length_ > dimension_length
        grid_dimensions_[2 * i + 1] += (box_length_ - r);
      }
    }

    // Calculate by how many boxes each dimension has grown
    int new_dimension_length = grid_dimensions_[1] - grid_dimensions_[0];
    int new_num_boxes = std::ceil(new_dimension_length / box_length_);
    int growth = new_num_boxes - num_boxes_axis_[0];

    if (growth > 0) {
      // Store the old number of boxes along each axis for comparison
      std::array<size_t, 3> tmp_num_boxes_axis = num_boxes_axis_;

      // Increase number of boxes along axis accordingly
      num_boxes_axis_[0] += growth;
      num_boxes_axis_[1] += growth;
      num_boxes_axis_[2] += growth;

      // We need to maintain the parity of the number of boxes along each
      // dimension, otherwise copying of the substances to the increases grid
      // will not be symmetrically done; resulting in shifting of boxes
      // We add a box in the negative direction, because the only way the parity
      // could have changed is because of adding a box in the positive direction
      // (due to the grid not being perfectly divisible; see above)
      if (num_boxes_axis_[0] % 2 != parity_) {
        for (int i = 0; i < 3; i++) {
          grid_dimensions_[2 * i] -= box_length_;
          num_boxes_axis_[i]++;
        }
      }

      // Temporarily save previous grid data
      auto tmp_c1 = c1_;
      auto tmp_gradients = gradients_;

      c1_.clear();
      c2_.clear();
      gradients_.clear();

      total_num_boxes_ =
          num_boxes_axis_[0] * num_boxes_axis_[1] * num_boxes_axis_[2];

      CopyOldData(tmp_c1, tmp_gradients, tmp_num_boxes_axis);

      assert(total_num_boxes_ >= tmp_num_boxes_axis[0] * tmp_num_boxes_axis[1] *
                                     tmp_num_boxes_axis[2] &&
             "The diffusion grid tried to shrink! It can only become larger");
    }

    // If we are utilising the Runge-Kutta method we need to resize an
    // additional vector, this will be used in estimating the concentration
    // between diffsuion steps.
    auto* param = Simulation::GetActive()->GetParam();
    if (param->diffusion_type_ == "RK") {
      r1_.resize(total_num_boxes_);
    }
  }

  /// Copies the concentration and gradients values to the new
  /// (larger) grid. In the 2D case it looks like the following:
  ///
  ///                             [0 0  0  0]
  ///               [v1 v2]  -->  [0 v1 v2 0]
  ///               [v3 v4]  -->  [0 v3 v4 0]
  ///                             [0 0  0  0]
  ///
  /// The dimensions are doubled in this case from 2x2 to 4x4
  /// If the dimensions would be increased from 2x2 to 3x3, it will still
  /// be increased to 4x4 in order for GetBoxIndex to function correctly
  ///
  void CopyOldData(const ParallelResizeVector<double>& old_c1,
                   const ParallelResizeVector<double>& old_gradients,
                   const std::array<size_t, 3>& old_num_boxes_axis) {
    // Allocate more memory for the grid data arrays
    c1_.resize(total_num_boxes_);
    c2_.resize(total_num_boxes_);
    gradients_.resize(3 * total_num_boxes_);

    auto incr_dim_x = num_boxes_axis_[0] - old_num_boxes_axis[0];
    auto incr_dim_y = num_boxes_axis_[1] - old_num_boxes_axis[1];
    auto incr_dim_z = num_boxes_axis_[2] - old_num_boxes_axis[2];

    int off_x = incr_dim_x / 2;
    int off_y = incr_dim_y / 2;
    int off_z = incr_dim_z / 2;

    int num_box_xy = num_boxes_axis_[0] * num_boxes_axis_[1];
    int old_box_xy = old_num_boxes_axis[0] * old_num_boxes_axis[1];
    int new_origin = off_z * (num_boxes_axis_[0] * num_boxes_axis_[1]) +
                     off_y * num_boxes_axis_[0] + off_x;
    for (size_t k = 0; k < old_num_boxes_axis[2]; k++) {
      int offset = new_origin + k * num_box_xy;
      for (size_t j = 0; j < old_num_boxes_axis[1]; j++) {
        if (j != 0) {
          offset += num_boxes_axis_[0];
        }
        for (size_t i = 0; i < old_num_boxes_axis[0]; i++) {
          auto idx = k * old_box_xy + j * old_num_boxes_axis[0] + i;
          c1_[offset + i] = old_c1[idx];
          gradients_[3 * (offset + i)] = old_gradients[3 * idx];
          gradients_[3 * (offset + i) + 1] = old_gradients[3 * idx + 1];
          gradients_[3 * (offset + i) + 2] = old_gradients[3 * idx + 2];
        }
      }
    }
  }

  /// Solves a 5-point stencil diffusion equation, with leaking-edge
  /// boundary conditions. Substances are allowed to leave the simulation
  /// space. This prevents building up concentration at the edges
  ///
  void DiffuseWithLeakingEdge() {
    int nx = num_boxes_axis_[0];
    int ny = num_boxes_axis_[1];
    int nz = num_boxes_axis_[2];

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
            c2_[c] =
                (dc_2_[0] * c1_[c] + dc_2_[1] * c1_[c - 1] +
                 dc_2_[2] * c1_[c + 1] + dc_2_[3] * c1_[s] + dc_2_[4] * c1_[n] +
                 dc_2_[5] * c1_[b] + dc_2_[6] * c1_[t]) *
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

  /// Solves a 5-point stencil diffusion equation, with closed-edge
  /// boundary conditions. Substances are not allowed to leave the simulation
  /// space. Keep in mind that the concentration can build up at the edges
  ///
  void DiffuseWithClosedEdge() {
    auto nx = num_boxes_axis_[0];
    auto ny = num_boxes_axis_[1];
    auto nz = num_boxes_axis_[2];

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
  void DiffuseEuler() {
    // check if diffusion coefficient and decay constant are 0
    // i.e. if we don't need to calculate diffusion update
    if (IsFixedSubstance()) {
      return;
    }

    const auto nx = num_boxes_axis_[0];
    const auto ny = num_boxes_axis_[1];
    const auto nz = num_boxes_axis_[2];

    const double ibl2 = 1 / (box_length_ * box_length_);
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
#pragma omp simd
          for (x = 1; x < nx - 1; x++) {
            ++c;
            ++n;
            ++s;
            ++b;
            ++t;

            if (y == 0 || y == (ny - 1) || z == 0 || z == (nz - 1)) {
              continue;
            }

            n = c - nx;
            s = c + nx;
            b = c - nx * ny;
            t = c + nx * ny;
            c2_[c] = (c1_[c] +
                      d * dt_ * (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1]) * ibl2 +
                      d * dt_ * (c1_[s] - 2 * c1_[c] + c1_[n]) * ibl2 +
                      d * dt_ * (c1_[b] - 2 * c1_[c] + c1_[t]) * ibl2) *
                     (1 - mu_);
          }
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  }

  void DiffuseEulerLeakingEdge() {
    // check if diffusion coefficient and decay constant are 0
    // i.e. if we don't need to calculate diffusion update
    if (IsFixedSubstance()) {
      return;
    }

    const auto nx = num_boxes_axis_[0];
    const auto ny = num_boxes_axis_[1];
    const auto nz = num_boxes_axis_[2];

    const double ibl2 = 1 / (box_length_ * box_length_);
    const double d = 1 - dc_[0];
    std::array<int, 4> l;

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

          c2_[c] = (c1_[c] + d * dt_ * (0 - 2 * c1_[c] + c1_[c + 1]) * ibl2 +
                    d * dt_ * (c1_[s] - 2 * c1_[c] + c1_[n]) * ibl2 +
                    d * dt_ * (c1_[b] - 2 * c1_[c] + c1_[t]) * ibl2) *
                   (1 - mu_);
#pragma omp simd
          for (x = 1; x < nx - 1; x++) {
            ++c;
            ++n;
            ++s;
            ++b;
            ++t;
            c2_[c] =
                (c1_[c] +
                 d * dt_ * (c1_[c - 1] - 2 * c1_[c] + c1_[c + 1]) * ibl2 +
                 d * dt_ * (l[0] * c1_[s] - 2 * c1_[c] + l[1] * c1_[n]) * ibl2 +
                 d * dt_ * (l[2] * c1_[b] - 2 * c1_[c] + l[3] * c1_[t]) *
                     ibl2) *
                (1 - mu_);
          }
          ++c;
          ++n;
          ++s;
          ++b;
          ++t;
          c2_[c] = (c1_[c] + d * dt_ * (c1_[c - 1] - 2 * c1_[c] + 0) * ibl2 +
                    d * dt_ * (c1_[s] - 2 * c1_[c] + c1_[n]) * ibl2 +
                    d * dt_ * (c1_[b] - 2 * c1_[c] + c1_[t]) * ibl2) *
                   (1 - mu_);
        }  // tile ny
      }    // tile nz
    }      // block ny
    c1_.swap(c2_);
  }

  void RK() {
    // check if diffusion coefficient and decay constant are 0
    // i.e. if we don't need to calculate diffusion update
    if (IsFixedSubstance()) {
      return;
    }

    const auto nx = num_boxes_axis_[0];
    const auto ny = num_boxes_axis_[1];
    const auto nz = num_boxes_axis_[2];

    const double ibl2 = 1 / (box_length_ * box_length_);
    const double d = 1 - dc_[0];
    double step = diffusion_step_;
    double h = dt_ / step;
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
                ++n;
                ++s;
                ++b;
                ++t;

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
              ++c;
              ++n;
              ++s;
              ++b;
              ++t;
            }  // tile ny
          }    // tile nz
        }      // block ny
      }
      c1_.swap(c2_);
    }
  }

  void RKLeaking() {
    // check if diffusion coefficient and decay constant are 0
    // i.e. if we don't need to calculate diffusion update
    if (IsFixedSubstance()) {
      return;
    }

    const auto nx = num_boxes_axis_[0];
    const auto ny = num_boxes_axis_[1];
    const auto nz = num_boxes_axis_[2];

    const double ibl2 = 1 / (box_length_ * box_length_);
    const double d = 1 - dc_[0];
    std::array<int, 6> l;

    double step = diffusion_step_;
    double h = dt_ / step;
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
              int c, cm, cp, n, s, b, t;
              c = x + y * nx + z * nx * ny;

              l.fill(1);

              if (y == 0) {
                n = c;
                l[2] = 0;
              } else {
                n = c - nx;
              }

              if (y == ny - 1) {
                s = c;
                l[3] = 0;
              } else {
                s = c + nx;
              }

              if (z == 0) {
                b = c;
                l[4] = 0;
              } else {
                b = c - nx * ny;
              }

              if (z == nz - 1) {
                t = c;
                l[5] = 0;
              } else {
                t = c + nx * ny;
              }

#pragma omp simd
              for (x = 1; x < nx - 1; x++) {
                ++c;
                ++n;
                ++s;
                ++b;
                ++t;

                if (x == 0) {
                  cm = c;
                  l[0] = 0;
                } else {
                  cm = c - 1;
                }

                if (y == ny - 1) {
                  cp = c;
                  l[1] = 0;
                } else {
                  cp = c + 1;
                }

                double h2 = h / 2.0;

                if (order == 0) {
                  k_[0] =
                      d * (l[0] * c1_[cm] - 2 * c1_[c] + l[1] * c1_[cp]) *
                          ibl2 +
                      d * (l[2] * c1_[s] - 2 * c1_[c] + l[3] * c1_[n]) * ibl2 +
                      d * (l[4] * c1_[b] - 2 * c1_[c] + l[5] * c1_[t]) * ibl2;
                  r1_[c] = c1_[c] + (k_[0] * h2);
                } else if (order == 1) {
                  k_[1] =
                      d * (l[0] * c1_[cm] - 2 * c1_[c] + l[1] * c1_[cp]) *
                          ibl2 +
                      d * (l[2] * c1_[s] - 2 * c1_[c] + l[3] * c1_[n]) * ibl2 +
                      d * (l[4] * c1_[b] - 2 * c1_[c] + l[5] * c1_[t]) * ibl2;

                  c2_[c] = c1_[c] + (k_[1] * h);
                }
              }
              ++c;
              ++n;
              ++s;
              ++b;
              ++t;
            }  // tile ny
          }    // tile nz
        }      // block ny
      }
      c1_.swap(c2_);
    }
  }

  /// Calculates the gradient for each box in the diffusion grid.
  /// The gradient is calculated in each direction (x, y, z) as following:
  ///
  /// c(x + box_length_) - c(x - box_length) / (2 * box_length_),
  ///
  /// where c(x) implies the concentration at position x
  ///
  /// At the edges the gradient is the same as the box next to it
  void CalculateGradient() {
    // check if gradient has been calculated once
    // and if diffusion coefficient and decay constant are 0
    // i.e. if we don't need to calculate gradient update
    if (init_gradient_ && IsFixedSubstance()) {
      return;
    }

    double gd = 1 / (box_length_ * 2);

    auto nx = num_boxes_axis_[0];
    auto ny = num_boxes_axis_[1];
    auto nz = num_boxes_axis_[2];

#pragma omp parallel for collapse(2)
    for (size_t z = 0; z < nz; z++) {
      for (size_t y = 0; y < ny; y++) {
        for (size_t x = 0; x < nx; x++) {
          int c, e, w, n, s, b, t;
          c = x + y * nx + z * nx * ny;

          if (x == 0) {
            e = c;
            w = c + 2;
          } else if (x == nx - 1) {
            e = c - 2;
            w = c;
          } else {
            e = c - 1;
            w = c + 1;
          }

          if (y == 0) {
            n = c + 2 * nx;
            s = c;
          } else if (y == ny - 1) {
            n = c;
            s = c - 2 * nx;
          } else {
            n = c + nx;
            s = c - nx;
          }

          if (z == 0) {
            t = c + 2 * nx * ny;
            b = c;
          } else if (z == nz - 1) {
            t = c;
            b = c - 2 * nx * ny;
          } else {
            t = c + nx * ny;
            b = c - nx * ny;
          }

          // Let the gradient point from low to high concentration
          gradients_[3 * c + 0] = (c1_[w] - c1_[e]) * gd;
          gradients_[3 * c + 1] = (c1_[n] - c1_[s]) * gd;
          gradients_[3 * c + 2] = (c1_[t] - c1_[b]) * gd;
        }
      }
    }
    if (!init_gradient_) {
      init_gradient_ = true;
    }
  }

  /// Increase the concentration at specified position with specified amount
  void IncreaseConcentrationBy(const Double3& position, double amount) {
    auto idx = GetBoxIndex(position);
    IncreaseConcentrationBy(idx, amount);
  }

  /// Increase the concentration at specified box with specified amount
  void IncreaseConcentrationBy(size_t idx, double amount) {
    assert(idx < total_num_boxes_ &&
           "Cell position is out of diffusion grid bounds");
    c1_[idx] += amount;
    if (c1_[idx] > concentration_threshold_) {
      c1_[idx] = concentration_threshold_;
    }
  }

  /// Get the concentration at specified position
  double GetConcentration(const Double3& position) const {
    return c1_[GetBoxIndex(position)];
  }

  /// Get the (normalized) gradient at specified position
  void GetGradient(const Double3& position, Double3* gradient) const {
    auto idx = GetBoxIndex(position);
    assert(idx < total_num_boxes_ &&
           "Cell position is out of diffusion grid bounds");
    (*gradient)[0] = gradients_[3 * idx];
    (*gradient)[1] = gradients_[3 * idx + 1];
    (*gradient)[2] = gradients_[3 * idx + 2];
    auto norm = std::sqrt((*gradient)[0] * (*gradient)[0] +
                          (*gradient)[1] * (*gradient)[1] +
                          (*gradient)[2] * (*gradient)[2]);
    if (norm > 1e-10) {
      (*gradient)[0] /= norm;
      (*gradient)[1] /= norm;
      (*gradient)[2] /= norm;
    }
  }

  std::array<uint32_t, 3> GetBoxCoordinates(const Double3& position) const {
    std::array<uint32_t, 3> box_coord;
    box_coord[0] = (floor(position[0]) - grid_dimensions_[0]) / box_length_;
    box_coord[1] = (floor(position[1]) - grid_dimensions_[2]) / box_length_;
    box_coord[2] = (floor(position[2]) - grid_dimensions_[4]) / box_length_;
    return box_coord;
  }

  size_t GetBoxIndex(const std::array<uint32_t, 3>& box_coord) const {
    size_t ret = box_coord[2] * num_boxes_axis_[0] * num_boxes_axis_[1] +
                 box_coord[1] * num_boxes_axis_[0] + box_coord[0];
    return ret;
  }

  /// Calculates the box index of the substance at specified position
  size_t GetBoxIndex(const Double3& position) const {
    auto box_coord = GetBoxCoordinates(position);
    return GetBoxIndex(box_coord);
  }

  void SetDiffusionSteps(int diffusion_step) {
    diffusion_step_ = diffusion_step;
  }

  void SetDecayConstant(double mu) { mu_ = mu; }

  void SetConcentrationThreshold(double t) { concentration_threshold_ = t; }

  double GetConcentrationThreshold() const { return concentration_threshold_; }

  const double* GetAllConcentrations() const { return c1_.data(); }

  const double* GetAllGradients() const { return gradients_.data(); }

  const std::array<size_t, 3>& GetNumBoxesArray() const {
    return num_boxes_axis_;
  }

  size_t GetNumBoxes() const { return total_num_boxes_; }

  double GetBoxLength() const { return box_length_; }

  int GetSubstanceId() const { return substance_; }

  const std::string& GetSubstanceName() const { return substance_name_; }

  double GetDecayConstant() const { return mu_; }

  const int32_t* GetDimensionsPtr() const { return grid_dimensions_.data(); }

  const std::array<int32_t, 6>& GetDimensions() const {
    return grid_dimensions_;
  }

  std::array<int32_t, 3> GetGridSize() const {
    std::array<int32_t, 3> ret;
    ret[0] = grid_dimensions_[1] - grid_dimensions_[0];
    ret[1] = grid_dimensions_[3] - grid_dimensions_[2];
    ret[2] = grid_dimensions_[5] - grid_dimensions_[4];
    return ret;
  }

  const std::array<double, 7>& GetDiffusionCoefficients() const { return dc_; }

  bool IsInitialized() const { return initialized_; }

  int GetResolution() const { return resolution_; }

  double GetBoxVolume() const { return box_volume_; }

  template <typename F>
  void AddInitializer(F function) {
    initializers_.push_back(function);
  }

  // retrun true if substance concentration and gradient don't evolve over time
  bool IsFixedSubstance() {
    return (mu_ == 0 && dc_[1] == 0 && dc_[2] == 0 && dc_[3] == 0 &&
            dc_[4] == 0 && dc_[5] == 0 && dc_[6] == 0);
  }

 private:
  /// The id of the substance of this grid
  int substance_ = 0;
  /// The name of the substance of this grid
  std::string substance_name_ = "";
  /// The side length of each box
  double box_length_ = 0;
  /// the volume of each box
  double box_volume_ = 0;
  /// The array of concentration values
  ParallelResizeVector<double> c1_ = {};
  /// An extra concentration data buffer for faster value updating
  ParallelResizeVector<double> c2_ = {};
  /// Buffers for Runge Kutta
  ParallelResizeVector<double> r1_ = {};
  /// k array for runge-kutta.
  std::array<double, 2> k_ = {};
  /// The array of gradients (x, y, z)
  ParallelResizeVector<double> gradients_ = {};
  /// The maximum concentration value that a box can have
  double concentration_threshold_ = 1e15;
  /// The diffusion coefficients [cc, cw, ce, cs, cn, cb, ct]
  std::array<double, 7> dc_ = {{0}};
  /// The timestep resolution fhe diffusion grid
  // TODO(ahmad): this probably needs to scale with Param::simulation_timestep
  double dt_ = 1.0;
  /// The decay constant
  double mu_ = 0;
  /// The grid dimensions of the diffusion grid
  std::array<int32_t, 6> grid_dimensions_ = {{0}};
  /// The number of boxes at each axis [x, y, z]
  std::array<size_t, 3> num_boxes_axis_ = {{0}};
  /// The total number of boxes in the diffusion grid
  size_t total_num_boxes_ = 0;
  /// Flag to determine if this grid has been initialized
  bool initialized_ = false;
  /// The resolution of the diffusion grid
  int resolution_ = 0;
  /// Number of steps for RK diffusion grid;
  unsigned int diffusion_step_ = 1;
  /// If false, grid dimensions are even; if true, they are odd
  bool parity_ = false;
  /// A list of functions that initialize this diffusion grid
  /// ROOT currently doesn't support IO of std::function
  std::vector<std::function<double(double, double, double)>> initializers_ =
      {};  //!
  // turn to true after gradient initialization
  bool init_gradient_ = false;

  BDM_CLASS_DEF_NV(DiffusionGrid, 1);
};

}  // namespace bdm

#endif  // CORE_DIFFUSION_GRID_H_
