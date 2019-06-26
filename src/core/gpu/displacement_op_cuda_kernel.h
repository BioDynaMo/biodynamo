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

class DisplacementOpCudaKernel {
 public:
  DisplacementOpCudaKernel(uint32_t num_objects, uint32_t num_boxes);
  virtual ~DisplacementOpCudaKernel();

  void LaunchDisplacementKernel(
      const double* positions, const double* diameter, const double* tractor_force,
      const double* adherence, uint32_t* box_id, const double* mass, const double* timestep,
      const double* max_displacement, const double* squared_radius, uint32_t* num_objects,
      uint32_t* starts, uint16_t* lengths, uint32_t* successors,
      uint32_t* box_length, uint32_t* num_boxes_axis, int32_t* grid_dimensions,
      double* cell_movements);

  void ResizeCellBuffers(uint32_t num_cells);
  void ResizeGridBuffers(uint32_t num_boxes);

 private:
  double* d_positions_ = NULL;
  double* d_diameters_ = NULL;
  double* d_mass_ = NULL;
  double* d_timestep_ = NULL;
  double* d_max_displacement_ = NULL;
  double* d_squared_radius_ = NULL;
  uint32_t* d_num_objects_ = NULL;
  double* d_cell_movements_ = NULL;
  double* d_tractor_force_ = NULL;
  double* d_adherence_ = NULL;
  uint32_t* d_box_id_ = NULL;
  uint32_t* d_starts_ = NULL;
  uint16_t* d_lengths_ = NULL;
  uint32_t* d_successors_ = NULL;
  uint32_t* d_box_length_ = NULL;
  uint32_t* d_num_boxes_axis_ = NULL;
  int32_t* d_grid_dimensions_ = NULL;
};

}  // namespace bdm

#endif  // CORE_GPU_DISPLACEMENT_OP_CUDA_KERNEL_H_
