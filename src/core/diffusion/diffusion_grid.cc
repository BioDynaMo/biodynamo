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

/// Transforms a BoundaryConditionType to the corresponding string
std::string BoundaryTypeToString(const BoundaryConditionType& type) {
  switch (type) {
    case BoundaryConditionType::kNeumann:
      return "Neumann";
    case BoundaryConditionType::kOpenBoundaries:
      return "open";
    case BoundaryConditionType::kClosedBoundaries:
      return "closed";
    case BoundaryConditionType::kDirichlet:
      return "Dirichlet";
    default:
      return "unknown";
  }
}

/// Transforms a string to the corresponding BoundaryConditionType
BoundaryConditionType StringToBoundaryType(const std::string& type) {
  if (type == "Neumann") {
    return BoundaryConditionType::kNeumann;
  } else if (type == "open") {
    return BoundaryConditionType::kOpenBoundaries;
  } else if (type == "closed") {
    return BoundaryConditionType::kClosedBoundaries;
  } else if (type == "Dirichlet") {
    return BoundaryConditionType::kDirichlet;
  } else {
    Log::Fatal("StringToBoundaryType", "Unknown boundary type: ", type);
    return BoundaryConditionType::kNeumann;
  }
}

DiffusionGrid::DiffusionGrid(int substance_id,
                             const std::string& substance_name, real_t dc,
                             real_t mu, int resolution)
    : dc_({{1 - dc, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6, dc / 6}}),
      mu_(mu),
      resolution_(resolution),
      boundary_condition_(std::make_unique<ConstantBoundaryCondition>(0.0)) {
  // Compatibility with new abstract interface
  SetContinuumId(substance_id);
  SetContinuumName(substance_name);

  // Get the default boundary type (set via parameter file)
  auto* param = Simulation::GetActive()->GetParam();
  bc_type_ = StringToBoundaryType(param->diffusion_boundary_condition);
}

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
  if (bounds[0] > bounds[1]) {
    Log::Fatal("DiffusionGrid::Initialize",
               "The grid dimensions are not correct. Lower bound is greater",
               " than upper bound. (substance '", GetContinuumName(), "')");
  }

  auto adjusted_res =
      resolution_ == 1 ? 2 : resolution_;  // avoid division by 0
  box_length_ = (grid_dimensions_[1] - grid_dimensions_[0]) /
                static_cast<real_t>(adjusted_res);

  // Check if box length is not too small
  if (box_length_ <= 1e-13) {
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

  // Print Info
  initialized_ = true;
  if (print_info_with_initialization_) {
    PrintInfo();
  }
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
  if (bc_type_ == BoundaryConditionType::kClosedBoundaries) {
    DiffuseWithClosedEdge(dt);
  } else if (bc_type_ == BoundaryConditionType::kOpenBoundaries) {
    DiffuseWithOpenEdge(dt);
  } else if (bc_type_ == BoundaryConditionType::kDirichlet) {
    DiffuseWithDirichlet(dt);
  } else if (bc_type_ == BoundaryConditionType::kNeumann) {
    DiffuseWithNeumann(dt);
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
      "this is likely to cause unphysical effects at the boundary. But if your "
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
  // If there are no initializers, we don't need to do anything and can return
  // immediately
  if (initializers_.empty()) {
    return;
  }

  // Define variables for loop bounds & boundary condition check
  const auto kNumBoxes = total_num_boxes_;
  const auto kGridSize = resolution_;
  // For certain boudaries, we also need to copy the values to the c2_ array
  const bool kCopyToC2 =
      (bc_type_ == BoundaryConditionType::kDirichlet ||
       bc_type_ == BoundaryConditionType::kClosedBoundaries ||
       bc_type_ == BoundaryConditionType::kOpenBoundaries);

// Apply all functions that initialize this diffusion grid
#pragma omp parallel for schedule(static)
  for (size_t idx = 0; idx < kNumBoxes; idx++) {
    // Determine the coordinates of the box
    const std::array<uint32_t, 3> box_coord = GetBoxCoordinates(idx);

    // Determine the real coordinates of the box
    std::array<real_t, 3> real_coord;
#pragma omp simd
    for (size_t i = 0; i < 3; i++) {
      real_coord[i] = grid_dimensions_[0] +
                      static_cast<real_t>(box_coord[i]) * box_length_ +
                      box_length_ / 2.0;
    }

    // Calculate the value of the substance in the box
    real_t value{0};
    if (bc_type_ == BoundaryConditionType::kDirichlet &&
        (box_coord[0] == 0 || box_coord[0] == kGridSize - 1 ||
         box_coord[1] == 0 || box_coord[1] == kGridSize - 1 ||
         box_coord[2] == 0 || box_coord[2] == kGridSize - 1)) {
      // Evaluate the boundary condition in case of Dirichlet boundary
      value = boundary_condition_->Evaluate(real_coord[0], real_coord[1],
                                            real_coord[2], 0);
    } else {
      for (size_t f = 0; f < initializers_.size(); f++) {
        value += initializers_[f](real_coord[0], real_coord[1], real_coord[2]);
      }
    }

    // Allow only lower_threshold_ <= value <= upper_threshold_
    value = std::max(value, lower_threshold_);
    value = std::min(value, upper_threshold_);

    // Set the value of the substance in the box
    c1_[idx] = value;

    // For certain boundaries, we need to copy the value to c2_
    if (kCopyToC2) {
      c2_[idx] = value;
    }
  }

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
  if (!precompute_gradients_) {
    return;
  }

#pragma omp parallel for collapse(2)
  for (uint32_t z = 0; z < resolution_; z++) {
    for (uint32_t y = 0; y < resolution_; y++) {
      for (uint32_t x = 0; x < resolution_; x++) {
        size_t idx = x + y * resolution_ + z * resolution_ * resolution_;
        const std::array<uint32_t, 3> box_coord = {x, y, z};
        // Get the neighboring boxes
        const auto neighbors = GetNeighboringBoxes(idx, box_coord);
        std::array<int, 6> comparison;  // array to determine discretization h
        std::transform(neighbors.begin(), neighbors.end(), comparison.begin(),
                       [idx](size_t n) { return (n == idx) ? 0 : 1; });

        // Calculate the gradient (no thread safety issues here)
        gradients_[idx][0] = (c1_[neighbors[1]] - c1_[neighbors[0]]) /
                             ((comparison[1] + comparison[0]) * box_length_);
        gradients_[idx][1] = (c1_[neighbors[3]] - c1_[neighbors[2]]) /
                             ((comparison[3] + comparison[2]) * box_length_);
        gradients_[idx][2] = (c1_[neighbors[5]] - c1_[neighbors[4]]) /
                             ((comparison[5] + comparison[4]) * box_length_);
      }
    }
  }
  if (!init_gradient_) {
    init_gradient_ = true;
  }
}

void DiffusionGrid::ChangeConcentrationBy(const Real3& position, real_t amount,
                                          InteractionMode mode,
                                          bool scale_with_resolution) {
  auto idx = GetBoxIndex(position);
  ChangeConcentrationBy(idx, amount, mode, scale_with_resolution);
}

/// Increase the concentration at specified box with specified amount
void DiffusionGrid::ChangeConcentrationBy(size_t idx, real_t amount,
                                          InteractionMode mode,
                                          bool scale_with_resolution) {
  if (idx >= total_num_boxes_) {
    Log::Error("DiffusionGrid::ChangeConcentrationBy",
               "You tried to change the concentration outside the bounds of "
               "the diffusion grid! The change was ignored.");
    return;
  }
  if (scale_with_resolution) {
    // Convert from amount to concentration of substance by dividing by
    // volume of box
    amount /= box_length_ * box_length_ * box_length_;
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
  return GetConcentration(idx);
}

/// Get the concentration at specified voxel
real_t DiffusionGrid::GetConcentration(const size_t idx) const {
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
  assert(gradient != nullptr);
  auto idx = GetBoxIndex(position);
  if (idx >= total_num_boxes_) {
    Log::Error("DiffusionGrid::GetGradient",
               "You tried to get the gradient outside the bounds of "
               "the diffusion grid! Returning zero gradient.");
    return;
  }
  if (init_gradient_) {
    *gradient = gradients_[idx];
  } else {
    // Get the neighboring boxes
    const auto neighbors = GetNeighboringBoxes(idx);
    std::array<int, 6> comparison;  // array to determine discretization h
    std::transform(neighbors.begin(), neighbors.end(), comparison.begin(),
                   [idx](size_t n) { return (n == idx) ? 0 : 1; });

    // Calculate the gradient (GetConcentration for thread safety)
    const real_t x_minus = GetConcentration(neighbors[0]);
    const real_t x_plus = GetConcentration(neighbors[1]);
    const real_t y_minus = GetConcentration(neighbors[2]);
    const real_t y_plus = GetConcentration(neighbors[3]);
    const real_t z_minus = GetConcentration(neighbors[4]);
    const real_t z_plus = GetConcentration(neighbors[5]);

    real_t grad_x =
        (x_plus - x_minus) / ((comparison[1] + comparison[0]) * box_length_);
    real_t grad_y =
        (y_plus - y_minus) / ((comparison[3] + comparison[2]) * box_length_);
    real_t grad_z =
        (z_plus - z_minus) / ((comparison[5] + comparison[4]) * box_length_);

    *gradient = Real3({grad_x, grad_y, grad_z});
  }
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

  for (size_t i = 0; i < 3; i++) {
// Check if position is within boundaries
#ifndef NDEBUG
    assert((position[i] >= grid_dimensions_[0]) &&
           "You tried to get the box coordinates outside the bounds of the "
           "diffusion grid!");
    assert((position[i] <= grid_dimensions_[1]) &&
           "You tried to get the box coordinates outside the bounds of the "
           "diffusion grid!");
#endif  // NDEBUG
    // Get box coords (Note: conversion to uint32_t should be save for typical
    // grid sizes)
    box_coord[i] = static_cast<uint32_t>(
        std::floor((position[i] - grid_dimensions_[0]) / box_length_));
  }
  return box_coord;
}

std::array<uint32_t, 3> DiffusionGrid::GetBoxCoordinates(
    const size_t idx) const {
  // Resolution must be smaller than uint32_t max for the box coordinates to be
  // representable
  assert(resolution_ < std::numeric_limits<uint32_t>::max());
  std::array<uint32_t, 3> box_coord;
  box_coord[0] = static_cast<uint32_t>(idx % resolution_);
  box_coord[1] = static_cast<uint32_t>((idx / resolution_) % resolution_);
  box_coord[2] = static_cast<uint32_t>(idx / (resolution_ * resolution_));
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

std::array<size_t, 6> DiffusionGrid::GetNeighboringBoxes(size_t index) const {
  const auto box_coord = GetBoxCoordinates(index);
  return GetNeighboringBoxes(index, box_coord);
};

std::array<size_t, 6> DiffusionGrid::GetNeighboringBoxes(
    size_t index, const std::array<uint32_t, 3>& box_coord) const {
  std::array<size_t, 6> neighbors;
  neighbors[0] = (box_coord[0] == 0) ? index : index - 1;
  neighbors[1] = (box_coord[0] == resolution_ - 1) ? index : index + 1;
  neighbors[2] = (box_coord[1] == 0) ? index : index - resolution_;
  neighbors[3] =
      (box_coord[1] == resolution_ - 1) ? index : index + resolution_;
  neighbors[4] =
      (box_coord[2] == 0) ? index : index - resolution_ * resolution_;
  neighbors[5] = (box_coord[2] == resolution_ - 1)
                     ? index
                     : index + resolution_ * resolution_;
  return neighbors;
}

void DiffusionGrid::SetBoundaryCondition(
    std::unique_ptr<BoundaryCondition> bc) {
  boundary_condition_ = std::move(bc);
}

BoundaryCondition* DiffusionGrid::GetBoundaryCondition() const {
  return boundary_condition_.get();
}

void DiffusionGrid::SetBoundaryConditionType(BoundaryConditionType bc_type) {
  auto previous_bc = BoundaryTypeToString(bc_type_);
  auto new_bc = BoundaryTypeToString(bc_type);
  Log::Info("DiffusionGrid::SetBoundaryConditionType",
            "Changing boundary condition from ", previous_bc, " to ", new_bc);
  bc_type_ = bc_type;
}

BoundaryConditionType DiffusionGrid::GetBoundaryConditionType() const {
  return bc_type_;
}

void DiffusionGrid::PrintInfo(std::ostream& out) {
  auto continuum_name = GetContinuumName();
  if (!IsInitialized()) {
    // If the grid is not yet initialized, many of the variables have no valid
    // values. We print a warning and print the info after the grid is
    // initialized.
    out << "DiffusionGrid" << continuum_name
        << "is not yet initialized. Will print info after initialization."
        << std::endl;
    print_info_with_initialization_ = true;
    return;
  }

  // Get all the info
  auto box_length = GetBoxLength();
  auto diffusion_coefficient = 1 - GetDiffusionCoefficients()[0];
  auto decay = GetDecayConstant();
  auto max_dt =
      (box_length * box_length) / (decay + 12.0 * diffusion_coefficient) * 2.0;
  auto domain = GetDimensions();
  auto umin = GetLowerThreshold();
  auto umax = GetUpperThreshold();
  auto resolution = GetResolution();
  auto num_boxes = GetNumBoxes();

  // Print the info
  out << "DiffusionGrid: " << continuum_name << "\n";
  out << "    D          = " << diffusion_coefficient << "\n";
  out << "    decay      = " << decay << "\n";
  out << "    dx         = " << box_length << "\n";
  out << "    max(dt)    <= " << max_dt << "\n";
  out << "    bounds     : " << umin << " < c < " << umax << "\n";
  out << "    domain     : "
      << "[" << domain[0] << ", " << domain[1] << "] x [" << domain[2] << ", "
      << domain[3] << "] x [" << domain[4] << ", " << domain[5] << "]\n";
  out << "    resolution : " << resolution << " x " << resolution << " x "
      << resolution << "\n";
  out << "    num boxes  : " << num_boxes << "\n";
  out << "    boundary   : " << BoundaryTypeToString(bc_type_) << "\n";
};

void DiffusionGrid::ParametersCheck(real_t dt) {
  // We evaluate a stability condition derived via a von Neumann stability
  // analysis (https://en.wikipedia.org/wiki/Von_Neumann_stability_analysis,
  // accessed 2022-10-27) of the diffusion equation. In comparison to the
  // Wikipedia article, we use a 3D diffusion equation and also consider the
  // decay. We end up with the following result:
  const bool stability =
      ((mu_ + 12.0 * (1 - dc_[0]) / (box_length_ * box_length_)) * dt <= 2.0);
  if (!stability) {
    const double max_dt =
        2.0 / (mu_ + 12.0 * (1 - dc_[0])) * (box_length_ * box_length_);
    Log::Fatal(
        "DiffusionGrid", "Stability condition violated. ",
        "The specified parameters of the diffusion grid with substance [",
        GetContinuumName(),
        "] will result in unphysical behavior (diffusion coefficient = ",
        (1 - dc_[0]), ", resolution = ", resolution_,
        ", decay constant = ", mu_, ", dt = ", dt,
        "). For the given parameters, the time step must be smaller than ",
        max_dt, ". Please refer to the user guide for more information.");
  }

  // We also check if the decay constant is too large. This is not a stability
  // condition, but it may result in unphysical behavior, e.g. negative values
  // for the concentration. We obtain the condition by assuming that all
  // concentration values of the previous time step are positive and demanding
  // the same for the next time step. We arrive at the following condition:
  const bool decay_safety =
      (1 - (mu_ + 6 * (1 - dc_[0]) / (box_length_ * box_length_) * dt) >= 0);
  if (!decay_safety) {
    Log::Fatal(
        "DiffusionGrid", "Decay may overstep into negative regime. ",
        "The specified parameters of the diffusion grid with substance [",
        GetContinuumName(),
        "] may result in unphysical behavior (diffusion coefficient = ",
        (1 - dc_[0]), ", resolution = ", resolution_,
        ", decay constant = ", mu_, ", dt = ", dt,
        "). Please refer to the user guide for more information.");
  }
}

}  // namespace bdm
