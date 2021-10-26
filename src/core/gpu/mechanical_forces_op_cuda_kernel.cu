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

#include <thrust/device_free.h>
#include <thrust/device_malloc.h>
#include "samples/common/inc/helper_math.h"

#include <unistd.h>
#include <iostream>

#include "core/gpu/cuda_error_chk.h"
#include "core/gpu/cuda_timer.h"
#include "core/gpu/helper_math_double.h"
#include "core/gpu/mechanical_forces_op_cuda_kernel.h"

void printMemoryUsage() {
  size_t availableMemory, totalMemory, usedMemory;
  cudaMemGetInfo(&availableMemory, &totalMemory);
  usedMemory = totalMemory - availableMemory;
  std::cout << "Device memory: used " << usedMemory << " available "
            << availableMemory << " total " << totalMemory << std::endl;
}

__device__ double squared_euclidian_distance(double3* positions, uint32_t idx,
                                             uint32_t nidx) {
  auto diff = positions[idx] - positions[nidx];
  return dot(diff, diff);
}

__device__ int3 get_box_coordinates(uint32_t box_idx, uint3 num_boxes_axis) {
  int3 box_coord;
  box_coord.z = box_idx / (num_boxes_axis.x * num_boxes_axis.y);
  uint32_t remainder = box_idx % (num_boxes_axis.x * num_boxes_axis.y);
  box_coord.y = remainder / num_boxes_axis.x;
  box_coord.x = remainder % num_boxes_axis.x;
  return box_coord;
}

__device__ uint32_t get_box_id(int3 bc, uint3 num_boxes_axis) {
  return bc.z * num_boxes_axis.x * num_boxes_axis.y + bc.y * num_boxes_axis.x +
         bc.x;
}

__device__ void compute_force(double3* positions, double* diameters,
                              uint32_t idx, uint32_t nidx, double3* result) {
  double r1 = 0.5 * diameters[idx];
  double r2 = 0.5 * diameters[nidx];
  // We take virtual bigger radii to have a distant interaction, to get a
  // desired density.
  double additional_radius = 10.0 * 0.15;
  r1 += additional_radius;
  r2 += additional_radius;

  double3 comp = positions[idx] - positions[nidx];
  double center_distance = length(comp);

  // the overlap distance (how much one penetrates in the other)
  double delta = r1 + r2 - center_distance;

  if (delta < 0) {
    return;
  }

  // to avoid a division by 0 if the centers are (almost) at the same location
  if (center_distance < 0.00000001) {
    *result += make_double3(42.0, 42.0, 42.0);
    return;
  }

  // printf("Colliding cell [%d] and [%d]\n", idx, nidx);
  // printf("Delta for neighbor [%d] = %f\n", nidx, delta);

  // the force itself
  double r = (r1 * r2) / (r1 + r2);
  double gamma = 1;  // attraction coeff
  double k = 2;      // repulsion coeff
  double f = k * delta - gamma * sqrt(r * delta);

  double module = f / center_distance;
  *result += module * comp;
  // printf("%f, %f, %f\n", module * comp.x, module * comp.y, module * comp.z);
  // printf(
  //     "Force between cell (%u) [%f, %f, %f] & cell (%u) [%f, %f, %f] = %f,%f, "
  //     "%f\n",
  //     idx, positions[idx].x, positions[idx].y, positions[idx].z, nidx,
  //     positions[nidx].x, positions[nidx].y, positions[nidx].z, module * comp.x,
  //     module * comp.y, module * comp.z);
}

__device__ void force(double3* positions, double* diameters, uint32_t idx,
                      uint32_t start, uint16_t length, uint32_t* successors,
                      double squared_radius, double3* result) {
  uint32_t nidx = start;
  for (uint16_t nb = 0; nb < length; nb++) {
    if (nidx != idx) {
      if (squared_euclidian_distance(positions, idx, nidx) < squared_radius) {
        compute_force(positions, diameters, idx, nidx, result);
      }
    }
    // traverse linked-list
    nidx = successors[nidx];
  }
}

__global__ void collide(double3* positions, double* diameters,
                        double3* tractor_force, double* adherence,
                        uint32_t* box_id, double* mass, double timestep,
                        double max_displacement, double squared_radius,
                        uint32_t num_agents, uint32_t* starts,
                        uint16_t* lengths, uint64_t* timestamps,
                        uint64_t current_timestamp, uint32_t* successors,
                        uint3* num_boxes_axis, double3* result) {
  uint32_t tidx = blockIdx.x * blockDim.x + threadIdx.x;
  // if (tidx == 0) {
  //   printf("Positions = ");
  //   for (uint32_t i = 0; i < num_agents; i++) {
  //     printf("%f, %f, %f, ", positions[i].x, positions[i].y, positions[i].z);
  //   }
  //   printf("\nSuccessors = ");
  //   for (uint32_t i = 0; i < num_agents; i++) {
  //     printf("%u, ", successors[i]);
  //   }
  //   printf("\nbox_id = ");
  //   uint32_t num_boxes = num_boxes_axis->x * num_boxes_axis->y * num_boxes_axis->z;
  //   for (uint32_t i = 0; i < num_boxes; i++) {
  //     printf("%u, ", box_id[i]);
  //   }
  //   printf("\nTimestamps = ");
  //   for (uint32_t i = 0; i < num_boxes; i++) {
  //     printf("%u, ", timestamps[i]);
  //   }
  //   printf("\n");
  // }
  if (tidx < num_agents) {
    result[tidx] += timestep * tractor_force[tidx];

    double3 collision_force = make_double3(0, 0, 0);
    double3 movement_at_next_step = make_double3(0, 0, 0);

    // Moore neighborhood
    int3 box_coords = get_box_coordinates(box_id[tidx], *num_boxes_axis);
    for (int z = -1; z <= 1; z++) {
      for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
          uint32_t bidx =
              get_box_id(box_coords + make_int3(x, y, z), *num_boxes_axis);
          if (timestamps[bidx] == current_timestamp && lengths[bidx] != 0) {
            force(positions, diameters, tidx, starts[bidx], lengths[bidx],
                  successors, squared_radius, &collision_force);
          }
        }
      }
    }

    // Mass needs to non-zero!
    double mh = timestep / mass[tidx];
    // printf("mh = %f\n", mh);

    if (length(collision_force) > adherence[tidx]) {
      movement_at_next_step += collision_force * mh;
      // printf("collision_force = (%f, %f, %f)\n", collision_force.x,
      // collision_force.y, collision_force.z); printf("cell_movement (1) = (%f,
      // %f, %f)\n", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx +
      // 2]);

      if (length(collision_force) * mh > max_displacement) {
        movement_at_next_step = normalize(movement_at_next_step);
        movement_at_next_step *= max_displacement;
      }
      result[tidx] = movement_at_next_step;
      // printf("cell_movement (2) = (%f, %f, %f)\n", result[3*tidx + 0],
      // result[3*tidx + 1], result[3*tidx + 2]);
    }
  }
}

bdm::MechanicalForcesOpCudaKernel::MechanicalForcesOpCudaKernel(
    uint32_t num_agents, uint32_t num_boxes) {
  d_positions_.resize(num_agents);
  d_diameters_.resize(num_agents);
  d_tractor_force_.resize(num_agents);
  d_adherence_.resize(num_agents);
  d_box_id_.resize(num_agents);
  d_mass_.resize(num_agents);
  d_cell_movements_.resize(num_agents);
  d_successors_.resize(num_agents);
  d_starts_.resize(num_boxes);
  d_lengths_.resize(num_boxes);
  d_timestamps_.resize(num_boxes);
  d_num_boxes_axis_ = thrust::device_malloc<uint3>(1);
}

void bdm::MechanicalForcesOpCudaKernel::LaunchMechanicalForcesKernel(
    const double* positions, const double* diameters,
    const double* tractor_force, const double* adherence,
    const uint32_t* box_id, const double* mass, const double timestep,
    const double max_displacement, const double squared_radius,
    const uint32_t num_agents, uint32_t* starts, uint16_t* lengths,
    uint64_t* timestamps, uint64_t current_timestamp, uint32_t* successors,
    uint32_t* num_boxes_axis, double* cell_movements) {
  uint32_t num_boxes =
      num_boxes_axis[0] * num_boxes_axis[1] * num_boxes_axis[2];

  // clang-format off
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_positions_.data()), 		positions, 3 * num_agents * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_diameters_.data()), 		diameters, num_agents * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_tractor_force_.data()), 	tractor_force, 3 * num_agents * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_adherence_.data()),     adherence, num_agents * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_box_id_.data()), 		box_id, num_agents * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_mass_.data()), 				mass, num_agents * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_starts_.data()), 			starts, num_boxes * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_lengths_.data()), 			lengths, num_boxes * sizeof(uint16_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_timestamps_.data()), 			timestamps, num_boxes * sizeof(uint64_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_successors_.data()), 		successors, num_agents * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpyAsync(thrust::raw_pointer_cast(d_num_boxes_axis_), 	num_boxes_axis, 3 * sizeof(uint32_t), cudaMemcpyHostToDevice));
  // clang-format on

  int blockSize = 128;
  int minGridSize;
  int gridSize;

  // Get a near-optimal occupancy with the following thread organization
  cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, collide, 0,
                                     num_agents);
  gridSize = (num_agents + blockSize - 1) / blockSize;

  // printf("gridSize = %d  |  blockSize = %d\n", gridSize, blockSize);
  collide<<<gridSize, blockSize>>>(
      thrust::raw_pointer_cast(d_positions_.data()),
      thrust::raw_pointer_cast(d_diameters_.data()),
      thrust::raw_pointer_cast(d_tractor_force_.data()),
      thrust::raw_pointer_cast(d_adherence_.data()),
      thrust::raw_pointer_cast(d_box_id_.data()),
      thrust::raw_pointer_cast(d_mass_.data()), timestep, max_displacement,
      squared_radius, num_agents, thrust::raw_pointer_cast(d_starts_.data()),
      thrust::raw_pointer_cast(d_lengths_.data()),
      thrust::raw_pointer_cast(d_timestamps_.data()), current_timestamp,
      thrust::raw_pointer_cast(d_successors_.data()),
      thrust::raw_pointer_cast(d_num_boxes_axis_),
      thrust::raw_pointer_cast(d_cell_movements_.data()));

  cudaMemcpyAsync(cell_movements,
                  thrust::raw_pointer_cast(d_cell_movements_.data()),
                  3 * num_agents * sizeof(double), cudaMemcpyDeviceToHost);
}

void bdm::MechanicalForcesOpCudaKernel::Sync() const {
  cudaDeviceSynchronize();
}

void bdm::MechanicalForcesOpCudaKernel::ResizeCellBuffers(uint32_t num_cells) {
  d_positions_.resize(num_cells);
  d_diameters_.resize(num_cells);
  d_tractor_force_.resize(num_cells);
  d_adherence_.resize(num_cells);
  d_box_id_.resize(num_cells);
  d_mass_.resize(num_cells);
  d_successors_.resize(num_cells);
  d_cell_movements_.resize(num_cells);
}

void bdm::MechanicalForcesOpCudaKernel::ResizeGridBuffers(uint32_t num_boxes) {
  d_starts_.resize(num_boxes);
  d_lengths_.resize(num_boxes);
  d_timestamps_.resize(num_boxes);
}

bdm::MechanicalForcesOpCudaKernel::~MechanicalForcesOpCudaKernel() {
  thrust::device_free(d_num_boxes_axis_);
}
