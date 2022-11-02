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

#include "core/diffusion/diffusion_grid.h"
#include <mutex>
#include "core/environment/environment.h"
#include "core/simulation.h"
#include "core/util/log.h"

namespace bdm {

void DiffusionGrid::Initialize() {
  if (resolution_ == 0) {
    Log::Fatal("DiffusionGrid::Initialize",
               "Resolution cannot be zero. (substance '", GetContinuumName(),
               "')");
  }

  // Get neighbor grid dimensions
  auto* env = Simulation::GetActive()->GetEnvironment();
  auto bounds = env->GetDimensionThresholds();

  grid_dimensions_ = {bounds[0], bounds[1]};

  // Example: diffusion grid dimensions from 0-40 and resolution
  // of 4. Resolution must be adjusted otherwise one data pointer will be
  // missing.
  // Without adjustment:
  //   box_length_: 10
  //   data points {0, 10, 20, 30} - 40 will be misssing!
  // With adjustment
  //   box_length_: 13.3
  //   data points: {0, 13.3, 26.6, 39.9}
  auto adjusted_res =
      resolution_ == 1 ? 2 : resolution_;  // avoid division by 0
  box_length_ = (grid_dimensions_[1] - grid_dimensions_[0]) /
                static_cast<real_t>(adjusted_res - 1);
  // TODO(ahmad): parametrize the minimum box_length
  if (box_length_ <= 1e-15) {
    Log::Fatal("DiffusionGrid::Initialize",
               "The box length was found to be (close to) zero. Please check "
               "the parameters for substance '",
               GetContinuumName(), "'");
  }

  box_volume_ = box_length_ * box_length_ * box_length_;
  parity_ = resolution_ % 2;
  total_num_boxes_ = resolution_ * resolution_ * resolution_;

  // Allocate memory for the concentration and gradient arrays
  locks_.resize(total_num_boxes_);
  c1_.resize(total_num_boxes_);
  c2_.resize(total_num_boxes_);
  gradients_.resize(total_num_boxes_);
}

void DiffusionGrid::Diffuse(real_t dt) {
  // check if diffusion coefficient and decay constant are 0
  // i.e. if we don't need to calculate diffusion update
  if (IsFixedSubstance()) {
    return;
  }

  // Note down last timestep
  last_dt_ = dt;
  // Set timestep for this iteration.
  ParametersCheck(dt);

  auto* param = Simulation::GetActive()->GetParam();
  if (param->diffusion_boundary_condition == "closed") {
    DiffuseWithClosedEdge(dt);
  } else if (param->diffusion_boundary_condition == "open") {
    DiffuseWithOpenEdge(dt);
  } else {
    Log::Error(
        "EulerGrid::Diffuse", "Boundary condition of type '",
        param->diffusion_boundary_condition,
        "' is not implemented. Defaulting to 'closed' boundary condition");
  }
}

void DiffusionGrid::Update() {
  // Get neighbor grid dimensions
  auto* env = Simulation::GetActive()->GetEnvironment();
  auto bounds = env->GetDimensionThresholds();
  // Update the grid dimensions such that each dimension ranges from
  // {bounds[0] - bounds[1]}
  grid_dimensions_ = {bounds[0], bounds[1]};

  // If the grid is not perfectly divisible along each dimension by the
  // box length, extend the grid so that it is
  int dimension_length = bounds[1] - bounds[0];
  for (int i = 0; i < 1; i++) {
    int r = fmod(dimension_length, box_length_);
    if (r > 1e-9) {
      // std::abs for the case that box_length_ > dimension_length
      grid_dimensions_[2 * i + 1] += (box_length_ - r);
    }
  }

  // Calculate new_dimension_length and new_resolution
  int new_dimension_length = grid_dimensions_[1] - grid_dimensions_[0];
  size_t new_resolution = std::ceil(new_dimension_length / box_length_);

  if (new_resolution > resolution_) {
    // Store the old number of boxes along each axis for comparison
    size_t tmp_resolution = resolution_;

    // Set new resolution
    resolution_ = new_resolution;

    // We need to maintain the parity of the number of boxes along each
    // dimension, otherwise copying of the substances to the increases grid
    // will not be symmetrically done; resulting in shifting of boxes
    // We add a box in the negative direction, because the only way the parity
    // could have changed is because of adding a box in the positive direction
    // (due to the grid not being perfectly divisible; see above)
    if (resolution_ % 2 != parity_) {
      grid_dimensions_[0] -= box_length_;
      resolution_++;
    }

    // Temporarily save previous grid data
    auto tmp_c1 = c1_;
    auto tmp_gradients = gradients_;

    c1_.clear();
    c2_.clear();
    gradients_.clear();

    total_num_boxes_ = resolution_ * resolution_ * resolution_;

    CopyOldData(tmp_c1, tmp_gradients, tmp_resolution);
  }
}

void DiffusionGrid::CopyOldData(
    const ParallelResizeVector<real_t>& old_c1,
    const ParallelResizeVector<Real3>& old_gradients, size_t old_resolution) {
  // Allocate more memory for the grid data arrays
  locks_.resize(total_num_boxes_);
  c1_.resize(total_num_boxes_);
  c2_.resize(total_num_boxes_);
  gradients_.resize(total_num_boxes_);

  Log::Warning(
      "DiffusionGrid::CopyOldData",
      "The size of the diffusion grid "
      "increased. BioDynaMo adds a halo around the domain filled with zeros. "
      "Depending your use-case, this might or might not be what you want. If "
      "you have non-zero concentrations / temperatures in the surrounding, "
      "this is likely to cause unphysical effects at the boudary. But if your "
      "grid values are mostly zero this is likely to work fine. Evaluate your "
      "results carefully.");

  auto incr_num_boxes = resolution_ - old_resolution;
  int off_dim = incr_num_boxes / 2;

  int num_box_xy = resolution_ * resolution_;
  int old_box_xy = old_resolution * old_resolution;
  int new_origin = off_dim * num_box_xy + off_dim * resolution_ + off_dim;
  for (size_t k = 0; k < old_resolution; k++) {
    int offset = new_origin + k * num_box_xy;
    for (size_t j = 0; j < old_resolution; j++) {
      if (j != 0) {
        offset += resolution_;
      }
      for (size_t i = 0; i < old_resolution; i++) {
        auto idx = k * old_box_xy + j * old_resolution + i;
        c1_[offset + i] = old_c1[idx];
        gradients_[offset + i] = old_gradients[idx];
      }
    }
  }
  // TODO: here we also need to copy c1_ into c2_ such that the boundaries are
  // equivalent once we introduce non-zero halos.
}

void DiffusionGrid::RunInitializers() {
  if (initializers_.empty()) {
    return;
  }

  auto nx = resolution_;
  auto ny = resolution_;
  auto nz = resolution_;

  // Apply all functions that initialize this diffusion grid
  for (size_t f = 0; f < initializers_.size(); f++) {
    for (uint32_t x = 0; x < nx; x++) {
      real_t real_x = grid_dimensions_[0] + x * box_length_;
      for (uint32_t y = 0; y < ny; y++) {
        real_t real_y = grid_dimensions_[0] + y * box_length_;
        for (uint32_t z = 0; z < nz; z++) {
          real_t real_z = grid_dimensions_[0] + z * box_length_;
          std::array<uint32_t, 3> box_coord = {x, y, z};
          size_t idx = GetBoxIndex(box_coord);
          real_t value = initializers_[f](real_x, real_y, real_z);
          ChangeConcentrationBy(idx, value);
        }
      }
    }
  }
  // Copy data to second array to ensure valid Dirichlet Boundary Conditions
  c2_ = c1_;

  // Clear the initializer to free up space
  initializers_.clear();
  initializers_.shrink_to_fit();
}

void DiffusionGrid::CalculateGradient() {
  // check if gradient has been calculated once
  // and if diffusion coefficient and decay constant are 0
  // i.e. if we don't need to calculate gradient update
  if (init_gradient_ && IsFixedSubstance()) {
    return;
  }

  real_t gd = 1 / (box_length_ * 2);

  auto nx = resolution_;
  auto ny = resolution_;
  auto nz = resolution_;

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
        gradients_[c][0] = (c1_[w] - c1_[e]) * gd;
        gradients_[c][1] = (c1_[n] - c1_[s]) * gd;
        gradients_[c][2] = (c1_[t] - c1_[b]) * gd;
      }
    }
  }
  if (!init_gradient_) {
    init_gradient_ = true;
  }
}

void DiffusionGrid::ChangeConcentrationBy(const Real3& position, real_t amount,
                                          InteractionMode mode) {
  auto idx = GetBoxIndex(position);
  ChangeConcentrationBy(idx, amount, mode);
}

/// Increase the concentration at specified box with specified amount
void DiffusionGrid::ChangeConcentrationBy(size_t idx, real_t amount,
                                          InteractionMode mode) {
  if (idx >= total_num_boxes_) {
    Log::Error("DiffusionGrid::ChangeConcentrationBy",
               "You tried to change the concentration outside the bounds of "
               "the diffusion grid! The change was ignored.");
    return;
  }
  std::lock_guard<Spinlock> guard(locks_[idx]);
  assert(idx < locks_.size());
  switch (mode) {
    case InteractionMode::kAdditive:
      c1_[idx] += amount;
      break;
    case InteractionMode::kExponential:
      c1_[idx] *= amount;
      break;
    case InteractionMode::kLogistic:
      c1_[idx] += ((amount > 0) ? upper_threshold_ - c1_[idx]
                                : c1_[idx] - lower_threshold_) *
                  amount;
      break;
    default:
      Log::Fatal("DiffusionGrid::ChangeConcentrationBy",
                 "Unknown interaction mode!");
  }

  // Enforce upper and lower bounds. (use std::clamp() when moving to C++17)
  if (c1_[idx] > upper_threshold_) {
    c1_[idx] = upper_threshold_;
  } else if (c1_[idx] < lower_threshold_) {
    c1_[idx] = lower_threshold_;
  } else {
    // c1_[idx] is bounded by the thresholds and does not need to be modified
  }
}

/// Get the concentration at specified position
real_t DiffusionGrid::GetValue(const Real3& position) const {
  auto idx = GetBoxIndex(position);
  if (idx >= total_num_boxes_) {
    Log::Error("DiffusionGrid::ChangeConcentrationBy",
               "You tried to get the concentration outside the bounds of "
               "the diffusion grid!");
    return 0;
  }
  assert(idx < locks_.size());
  std::lock_guard<Spinlock> guard(locks_[idx]);
  return c1_[idx];
}

void DiffusionGrid::GetGradient(const Real3& position, Real3* gradient,
                                bool normalize) const {
  auto idx = GetBoxIndex(position);
  if (idx >= total_num_boxes_) {
    Log::Error("DiffusionGrid::GetGradient",
               "You tried to get the gradient outside the bounds of "
               "the diffusion grid! Returning zero gradient.");
    return;
  }
  *gradient = gradients_[idx];
  if (normalize) {
    auto norm = gradient->Norm();
    if (norm > 1e-10) {
      gradient->Normalize(norm);
    }
  }
}

std::array<uint32_t, 3> DiffusionGrid::GetBoxCoordinates(
    const Real3& position) const {
  std::array<uint32_t, 3> box_coord;
  box_coord[0] = (floor(position[0]) - grid_dimensions_[0]) / box_length_;
  box_coord[1] = (floor(position[1]) - grid_dimensions_[0]) / box_length_;
  box_coord[2] = (floor(position[2]) - grid_dimensions_[0]) / box_length_;
  return box_coord;
}

size_t DiffusionGrid::GetBoxIndex(
    const std::array<uint32_t, 3>& box_coord) const {
  size_t ret = box_coord[2] * resolution_ * resolution_ +
               box_coord[1] * resolution_ + box_coord[0];
  return ret;
}

/// Calculates the box index of the substance at specified position
size_t DiffusionGrid::GetBoxIndex(const Real3& position) const {
  auto box_coord = GetBoxCoordinates(position);
  return GetBoxIndex(box_coord);
}

void DiffusionGrid::ParametersCheck(real_t dt) {
  if ((((1 - dc_[0]) * dt) / (box_length_ * box_length_) >= (1.0 / 6)) ||
      ((mu_ * dt) > 1.0)) {
    Log::Fatal(
        "DiffusionGrid",
        "The specified parameters of the diffusion grid with substance [",
        GetContinuumName(),
        "] will result in unphysical behavior (diffusion coefficient = ",
        (1 - dc_[0]), ", resolution = ", resolution_,
        ", decay constant * dt = ", mu_ * dt,
        "). Please refer to the user guide for more information.");
  }
}
}  // namespace bdm
