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

#ifndef CORE_GPU_MECHANICAL_FORCES_OP_CUDA_KERNEL_H_
#define CORE_GPU_MECHANICAL_FORCES_OP_CUDA_KERNEL_H_

#include <math.h>
#include <stdint.h>
#include "stdio.h"

#ifdef USE_CUDA
#include <thrust/device_ptr.h>
#include <thrust/device_vector.h>
#endif

namespace bdm {

class MechanicalForcesOpCudaKernel {
 public:
  MechanicalForcesOpCudaKernel(uint32_t num_agents, uint32_t num_boxes);
  virtual ~MechanicalForcesOpCudaKernel();

  void LaunchMechanicalForcesKernel(
      const double* positions, const double* diameter,
      const double* tractor_force, const double* adherence,
      const uint32_t* box_id, const double* mass, const double timestep,
      const double max_displacement, const double squared_radius,
      const uint32_t num_agents, uint32_t* starts, uint16_t* lengths,
      uint64_t* timestamps, uint64_t current_timestamp, uint32_t* successors,
      uint32_t* num_boxes_axis, double* cell_movements);

  void Sync() const;
  void ResizeCellBuffers(uint32_t num_cells);
  void ResizeGridBuffers(uint32_t num_boxes);

#ifdef USE_CUDA
 private:
  thrust::device_vector<double3> d_positions_;
  thrust::device_vector<double3> d_tractor_force_;
  thrust::device_vector<double> d_adherence_;
  thrust::device_vector<double> d_diameters_;
  thrust::device_vector<double> d_mass_;
  thrust::device_vector<double3> d_cell_movements_;
  thrust::device_vector<uint32_t> d_box_id_;
  thrust::device_vector<uint32_t> d_starts_;
  thrust::device_vector<uint16_t> d_lengths_;
  thrust::device_vector<uint32_t> d_successors_;
  thrust::device_vector<uint64_t> d_timestamps_;
  thrust::device_ptr<uint3> d_num_boxes_axis_;
#endif  // USE_CUDA
};

#ifndef USE_CUDA
// Empty implementation if CUDA is not used to avoid undefined reference linking
// error.
//
inline MechanicalForcesOpCudaKernel::MechanicalForcesOpCudaKernel(
    uint32_t num_agents, uint32_t num_boxes) {}
inline MechanicalForcesOpCudaKernel::~MechanicalForcesOpCudaKernel() = default;

inline void MechanicalForcesOpCudaKernel::LaunchMechanicalForcesKernel(
    const double* positions, const double* diameter,
    const double* tractor_force, const double* adherence,
    const uint32_t* box_id, const double* mass, const double timestep,
    const double max_displacement, const double squared_radius,
    const uint32_t num_agents, uint32_t* starts, uint16_t* lengths,
    uint64_t* timestamps, uint64_t current_timestamp, uint32_t* successors,
    uint32_t* num_boxes_axis, double* cell_movements) {}

inline void MechanicalForcesOpCudaKernel::Sync() const {}
inline void MechanicalForcesOpCudaKernel::ResizeCellBuffers(
    uint32_t num_cells) {}
inline void MechanicalForcesOpCudaKernel::ResizeGridBuffers(
    uint32_t num_boxes) {}

#endif  // USE_CUDA

}  // namespace bdm

#endif  // CORE_GPU_MECHANICAL_FORCES_OP_CUDA_KERNEL_H_
