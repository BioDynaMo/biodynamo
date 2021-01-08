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

#ifndef CORE_GPU_MECHANICAL_FORCES_OP_CUDA_KERNEL_H_
#define CORE_GPU_MECHANICAL_FORCES_OP_CUDA_KERNEL_H_

#include <math.h>
#include <stdint.h>
#include "stdio.h"

namespace bdm {

class MechanicalForcesOpCudaKernel {
 public:
  MechanicalForcesOpCudaKernel(uint32_t num_objects, uint32_t num_boxes);
  virtual ~MechanicalForcesOpCudaKernel();

  void LaunchMechanicalForcesKernel(
      const double* positions, const double* diameter,
      const double* tractor_force, const double* adherence,
      const uint32_t* box_id, const double* mass, const double* timestep,
      const double* max_displacement, const double* squared_radius,
      const uint32_t* num_objects, uint32_t* starts, uint16_t* lengths,
      uint64_t* timestamps, uint64_t* current_timestamp, uint32_t* successors,
      uint32_t* num_boxes_axis, double* cell_movements);

  void Sync() const;
  void ResizeCellBuffers(uint32_t num_cells);
  void ResizeGridBuffers(uint32_t num_boxes);

#ifdef USE_CUDA
 private:
  double* d_positions_ = nullptr;
  double* d_diameters_ = nullptr;
  double* d_mass_ = nullptr;
  double* d_timestep_ = nullptr;
  double* d_max_displacement_ = nullptr;
  double* d_squared_radius_ = nullptr;
  uint32_t* d_num_objects_ = nullptr;
  double* d_cell_movements_ = nullptr;
  double* d_tractor_force_ = nullptr;
  double* d_adherence_ = nullptr;
  uint32_t* d_box_id_ = nullptr;
  uint32_t* d_starts_ = nullptr;
  uint16_t* d_lengths_ = nullptr;
  uint64_t* d_timestamps_ = nullptr;
  uint64_t* d_current_timestamp_ = nullptr;
  uint32_t* d_successors_ = nullptr;
  uint32_t* d_num_boxes_axis_ = nullptr;
#endif  // USE_CUDA
};

#ifndef USE_CUDA
// Empty implementaiton if CUDA is not used to avoid undefined reference linking
// error.
//
inline MechanicalForcesOpCudaKernel::MechanicalForcesOpCudaKernel(
    uint32_t num_objects, uint32_t num_boxes) {}
inline MechanicalForcesOpCudaKernel::~MechanicalForcesOpCudaKernel() {}

inline void MechanicalForcesOpCudaKernel::LaunchMechanicalForcesKernel(
    const double* positions, const double* diameter,
    const double* tractor_force, const double* adherence,
    const uint32_t* box_id, const double* mass, const double* timestep,
    const double* max_displacement, const double* squared_radius,
    const uint32_t* num_objects, uint32_t* starts, uint16_t* lengths,
    uint64_t* timestamps, uint64_t* current_timestamp, uint32_t* successors,
    uint32_t* num_boxes_axis, double* cell_movements) {}

inline void MechanicalForcesOpCudaKernel::Sync() const {}
inline void MechanicalForcesOpCudaKernel::ResizeCellBuffers(
    uint32_t num_cells) {}
inline void MechanicalForcesOpCudaKernel::ResizeGridBuffers(
    uint32_t num_boxes) {}

#endif  // USE_CUDA

}  // namespace bdm

#endif  // CORE_GPU_MECHANICAL_FORCES_OP_CUDA_KERNEL_H_
