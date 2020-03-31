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

#ifndef CORE_GPU_DISPLACEMENT_OP_CUDA_KERNEL_H_
#define CORE_GPU_DISPLACEMENT_OP_CUDA_KERNEL_H_

#include <math.h>
#include <stdint.h>
#include "stdio.h"

namespace bdm {

// Bitmask to verify if NeuriteElement has daughter(s) and/or mother
constexpr uint8_t kHasDaughterLeft = 1;
constexpr uint8_t kHasDaughterRight = 2;
constexpr uint8_t kHasMother = 4;

class DisplacementOpCudaKernel {
 public:
  DisplacementOpCudaKernel(uint32_t num_objects, uint32_t num_boxes);
  virtual ~DisplacementOpCudaKernel();

  void LaunchDisplacementKernel(
      const uint8_t* shape, const double* positions, const double* diameters,
      const double* tractor_force, const double* adherence,
      const uint32_t* box_id, const double* mass, const double* ne_proximal_end,
      const double* ne_distal_end, const double* ne_axis,
      const double* ne_tension,
      const double* ne_force_to_transmit_to_proximal_mass,
      const uint32_t* daughter_left, const uint32_t* daughter_right,
      const uint32_t* mother, const uint8_t* has_daughter_or_mother,
      const double* timestep, const double* max_displacement,
      const double* squared_radius, const uint32_t* num_objects,
      uint32_t* starts, uint16_t* lengths, uint64_t* timestamps,
      uint64_t* current_timestamp, uint32_t* successors, uint32_t* box_length,
      uint32_t* num_boxes_axis, int32_t* grid_dimensions,
      double* cell_movements, double* force_to_transmit_to_proximal_mass);

  void ResizeCellBuffers(uint32_t num_cells);
  void ResizeGridBuffers(uint32_t num_boxes);

 private:
  uint8_t* d_shape_ = nullptr;
  double* d_positions_ = nullptr;
  double* d_diameters_ = nullptr;
  double* d_mass_ = nullptr;
  double* d_timestep_ = nullptr;
  double* d_max_displacement_ = nullptr;
  double* d_squared_radius_ = nullptr;
  uint32_t* d_num_objects_ = nullptr;
  double* d_cell_movements_ = nullptr;
  double* d_force_to_transmit_to_proximal_mass_ = nullptr;
  double* d_ne_force_to_transmit_to_proximal_mass_ = nullptr;
  double* d_tractor_force_ = nullptr;
  double* d_adherence_ = nullptr;
  double* d_ne_proximal_end_ = nullptr;
  double* d_ne_distal_end_ = nullptr;
  double* d_ne_axis_ = nullptr;
  double* d_ne_tension_ = nullptr;
  uint32_t* d_daughter_left_ = nullptr;
  uint32_t* d_daughter_right_ = nullptr;
  uint32_t* d_mother_ = nullptr;
  uint8_t* d_has_daughter_or_mother_ = nullptr;
  uint32_t* d_box_id_ = nullptr;
  uint32_t* d_starts_ = nullptr;
  uint16_t* d_lengths_ = nullptr;
  uint64_t* d_timestamps_ = nullptr;
  uint64_t* d_current_timestamp_ = nullptr;
  uint32_t* d_successors_ = nullptr;
  uint32_t* d_box_length_ = nullptr;
  uint32_t* d_num_boxes_axis_ = nullptr;
  int32_t* d_grid_dimensions_ = nullptr;
};

}  // namespace bdm

#endif  // CORE_GPU_DISPLACEMENT_OP_CUDA_KERNEL_H_
