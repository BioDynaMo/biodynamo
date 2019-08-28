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

#include "samples/common/inc/helper_math.h"
#include "core/gpu/displacement_op_cuda_kernel.h"

#define GpuErrchk(ans) { GpuAssert((ans), __FILE__, __LINE__); }
inline void GpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (code == cudaErrorInsufficientDriver) {
        printf("This probably means that no CUDA-compatible GPU has been detected. Consider setting the use_opencl flag to \"true\" in the bmd.toml file to use OpenCL instead.\n");
      }
      if (abort) exit(code);
   }
}

__device__ double norm(double3 v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

__device__ double squared_euclidian_distance(double* positions, uint32_t idx, uint32_t nidx) {
  const double dx = positions[3*idx + 0] - positions[3*nidx + 0];
  const double dy = positions[3*idx + 1] - positions[3*nidx + 1];
  const double dz = positions[3*idx + 2] - positions[3*nidx + 2];
  return (dx * dx + dy * dy + dz * dz);
}

__device__ int3 get_box_coordinates(double3 pos, int32_t* grid_dimensions, uint32_t box_length) {
  int3 box_coords;
  box_coords.x = (floor(pos.x) - grid_dimensions[0]) / box_length;
  box_coords.y = (floor(pos.y) - grid_dimensions[1]) / box_length;
  box_coords.z = (floor(pos.z) - grid_dimensions[2]) / box_length;
  return box_coords;
}

__device__ int3 get_box_coordinates_2(uint32_t box_idx, uint32_t* num_boxes_axis_) {
  int3 box_coord;
  box_coord.z = box_idx / (num_boxes_axis_[0]*num_boxes_axis_[1]);
  uint32_t remainder = box_idx % (num_boxes_axis_[0]*num_boxes_axis_[1]);
  box_coord.y = remainder / num_boxes_axis_[0];
  box_coord.x = remainder % num_boxes_axis_[0];
  return box_coord;
}

__device__ uint32_t get_box_id_2(int3 bc, uint32_t* num_boxes_axis) {
  return bc.z * num_boxes_axis[0]*num_boxes_axis[1] + bc.y * num_boxes_axis[0] + bc.x;
}

__device__ uint32_t get_box_id(double3 pos, uint32_t* num_boxes_axis, int32_t* grid_dimensions, uint32_t box_length) {
  int3 box_coords = get_box_coordinates(pos, grid_dimensions, box_length);
  return get_box_id_2(box_coords, num_boxes_axis);
}

__device__ void compute_force(double* positions, double* diameters, uint32_t idx, uint32_t nidx, double3* result) {
  double r1 = 0.5 * diameters[idx];
  double r2 = 0.5 * diameters[nidx];
  // We take virtual bigger radii to have a distant interaction, to get a desired density.
  double additional_radius = 10.0 * 0.15;
  r1 += additional_radius;
  r2 += additional_radius;

  double comp1 = positions[3*idx + 0] - positions[3*nidx + 0];
  double comp2 = positions[3*idx + 1] - positions[3*nidx + 1];
  double comp3 = positions[3*idx + 2] - positions[3*nidx + 2];
  double center_distance = sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);

  // the overlap distance (how much one penetrates in the other)
  double delta = r1 + r2 - center_distance;

  if (delta < 0) {
    return;
  }

  // to avoid a division by 0 if the centers are (almost) at the same location
  if (center_distance < 0.00000001) {
    result->x += 42.0;
    result->y += 42.0;
    result->z += 42.0;
    return;
  }

  // printf("Colliding cell [%d] and [%d]\n", idx, nidx);
  // printf("Delta for neighbor [%d] = %f\n", nidx, delta);

  // the force itself
  double r = (r1 * r2) / (r1 + r2);
  double gamma = 1; // attraction coeff
  double k = 2;     // repulsion coeff
  double f = k * delta - gamma * sqrt(r * delta);

  double module = f / center_distance;
  result->x += module * comp1;
  result->y += module * comp2;
  result->z += module * comp3;
  // printf("%f, %f, %f\n", module * comp1, module * comp2, module * comp3);
  // printf("Force between cell (%u) [%f, %f, %f] & cell (%u) [%f, %f, %f] = %f, %f, %f\n", idx, positions[3*idx + 0], positions[3*idx + 1], positions[3*idx + 2], nidx, positions[3*nidx + 0], positions[3*nidx + 1], positions[3*nidx + 2], module * comp1, module * comp2, module * comp3);
}

__device__ void default_force(double* positions,
                   double* diameters,
                   uint32_t idx, uint32_t start, uint16_t length,
                   uint32_t* successors,
                   double* squared_radius,
                   double3* result) {
  uint32_t nidx = start;
  for (uint16_t nb = 0; nb < length; nb++) {
    if (nidx != idx) {
      if (squared_euclidian_distance(positions, idx, nidx) < squared_radius[0]) {
        compute_force(positions, diameters, idx, nidx, result);
      }
    }
    // traverse linked-list
    nidx = successors[nidx];
  }
}

__global__ void collide(
       double* positions,
       double* diameters,
       double* tractor_force,
       double* adherence,
       uint32_t* box_id,
       double* mass,
       double* timestep,
       double* max_displacement,
       double* squared_radius,
       uint32_t* num_objects,
       uint32_t* starts,
       uint16_t* lengths,
       uint32_t* successors,
       uint32_t* box_length,
       uint32_t* num_boxes_axis,
       int32_t* grid_dimensions,
       double* result) {
  uint32_t tidx = blockIdx.x * blockDim.x + threadIdx.x;
  if (tidx < num_objects[0]) {
    result[3*tidx + 0] = timestep[0] * tractor_force[3*tidx + 0];
    result[3*tidx + 1] = timestep[0] * tractor_force[3*tidx + 1];
    result[3*tidx + 2] = timestep[0] * tractor_force[3*tidx + 2];
    // printf("cell_movement = (%f, %f, %f)\n", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);
    
    double3 collision_force = make_double3(0, 0, 0);

    // Moore neighborhood
    int3 box_coords = get_box_coordinates_2(box_id[tidx], num_boxes_axis);
    for (int z = -1; z <= 1; z++) {
      for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
          uint32_t bidx = get_box_id_2(box_coords + make_int3(x, y, z), num_boxes_axis);
          if (lengths[bidx] != 0) {
            default_force(positions, diameters, tidx, starts[bidx], lengths[bidx], successors, squared_radius, &collision_force);
          }
        }
      }
    }

    // Mass needs to non-zero!
    double mh = timestep[0] / mass[tidx];
    // printf("mh = %f\n", mh);

    if (norm(collision_force) > adherence[tidx]) {
      result[3*tidx + 0] += collision_force.x * mh;
      result[3*tidx + 1] += collision_force.y * mh;
      result[3*tidx + 2] += collision_force.z * mh;
      // printf("collision_force = (%f, %f, %f)\n", collision_force.x, collision_force.y, collision_force.z);
      // printf("cell_movement (1) = (%f, %f, %f)\n", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);

      if (norm(collision_force) * mh > max_displacement[0]) {
        result[3*tidx + 0] = max_displacement[0];
        result[3*tidx + 1] = max_displacement[0];
        result[3*tidx + 2] = max_displacement[0];
      }
      // printf("cell_movement (2) = (%f, %f, %f)\n", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);
    }
  }
}

bdm::DisplacementOpCudaKernel::DisplacementOpCudaKernel(uint32_t num_objects, uint32_t num_boxes) {
  // printf("num_objects = %u  |  num_boxes = %u\n", num_objects, num_boxes);
  GpuErrchk(cudaMalloc(&d_positions_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_diameters_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_tractor_force_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_adherence_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_box_id_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_mass_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_timestep_, sizeof(double)));
  GpuErrchk(cudaMalloc(&d_max_displacement_, sizeof(double)));
  GpuErrchk(cudaMalloc(&d_squared_radius_, sizeof(double)));
  GpuErrchk(cudaMalloc(&d_num_objects_, sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_starts_, num_boxes * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_lengths_, num_boxes * sizeof(uint16_t)));
  GpuErrchk(cudaMalloc(&d_successors_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_box_length_, sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_num_boxes_axis_, 3 * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_grid_dimensions_, 3 * sizeof(int32_t)));
  GpuErrchk(cudaMalloc(&d_cell_movements_, 3 * num_objects * sizeof(double)));
}

void bdm::DisplacementOpCudaKernel::LaunchDisplacementKernel(const double* positions,
    const double* diameters, const double* tractor_force, const double* adherence,
    uint32_t* box_id, const double* mass, const double* timestep, const double* max_displacement,
    const double* squared_radius, uint32_t* num_objects, uint32_t* starts,
    uint16_t* lengths, uint32_t* successors, uint32_t* box_length,
    uint32_t* num_boxes_axis, int32_t* grid_dimensions,
    double* cell_movements) {
  uint32_t num_boxes = num_boxes_axis[0] * num_boxes_axis[1] * num_boxes_axis[2];

  GpuErrchk(cudaMemcpy(d_positions_, 		positions, 3 * num_objects[0] * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_diameters_, 		diameters, num_objects[0] * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_tractor_force_, 	tractor_force, 3 * num_objects[0] * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_adherence_,     adherence, num_objects[0] * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_box_id_, 		box_id, num_objects[0] * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_mass_, 				mass, num_objects[0] * sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_timestep_, 			timestep, sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_max_displacement_,  max_displacement, sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_squared_radius_, 	squared_radius, sizeof(double), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_num_objects_, 				num_objects, sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_starts_, 			starts, num_boxes * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_lengths_, 			lengths, num_boxes * sizeof(uint16_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_successors_, 		successors, num_objects[0] * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_box_length_, 		box_length, sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_num_boxes_axis_, 	num_boxes_axis, 3 * sizeof(uint32_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_grid_dimensions_, 	grid_dimensions, 3 * sizeof(uint32_t), cudaMemcpyHostToDevice));

  int blockSize = 128;
  int minGridSize;
  int gridSize;

  // Get a near-optimal occupancy with the following thread organization
  cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, collide, 0, num_objects[0]);
  gridSize = (num_objects[0] + blockSize - 1) / blockSize;

  // printf("gridSize = %d  |  blockSize = %d\n", gridSize, blockSize);
  collide<<<gridSize, blockSize>>>(d_positions_, d_diameters_, d_tractor_force_,
    d_adherence_, d_box_id_, d_mass_, d_timestep_, d_max_displacement_,
    d_squared_radius_, d_num_objects_, d_starts_, d_lengths_, d_successors_,
    d_box_length_, d_num_boxes_axis_, d_grid_dimensions_, d_cell_movements_);

  // We need to wait for the kernel to finish before reading back the result
  cudaDeviceSynchronize();
  cudaMemcpy(cell_movements, d_cell_movements_, 3 * num_objects[0] * sizeof(double), cudaMemcpyDeviceToHost);
}

void bdm::DisplacementOpCudaKernel::ResizeCellBuffers(uint32_t num_cells) {
  cudaFree(d_positions_);
  cudaFree(d_diameters_);
  cudaFree(d_tractor_force_);
  cudaFree(d_adherence_);
  cudaFree(d_box_id_);
  cudaFree(d_mass_);
  cudaFree(d_successors_);
  cudaFree(d_cell_movements_);

  cudaMalloc(&d_positions_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_diameters_, num_cells * sizeof(double));
  cudaMalloc(&d_tractor_force_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_adherence_, num_cells * sizeof(double));
  cudaMalloc(&d_box_id_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_mass_, num_cells * sizeof(double));
  cudaMalloc(&d_successors_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_cell_movements_, 3 * num_cells * sizeof(double));
}

void bdm::DisplacementOpCudaKernel::ResizeGridBuffers(uint32_t num_boxes) {
  cudaFree(d_starts_);
  cudaFree(d_lengths_);

  cudaMalloc(&d_starts_, num_boxes * sizeof(uint32_t));
  cudaMalloc(&d_lengths_, num_boxes * sizeof(uint16_t));
}

bdm::DisplacementOpCudaKernel::~DisplacementOpCudaKernel() {
  cudaFree(d_positions_);
  cudaFree(d_diameters_);
  cudaFree(d_tractor_force_);
  cudaFree(d_adherence_);
  cudaFree(d_box_id_);
  cudaFree(d_mass_);
  cudaFree(d_timestep_);
  cudaFree(d_max_displacement_);
  cudaFree(d_squared_radius_);
  cudaFree(d_num_objects_);
  cudaFree(d_starts_);
  cudaFree(d_lengths_);
  cudaFree(d_successors_);
  cudaFree(d_num_boxes_axis_);
  cudaFree(d_grid_dimensions_);
  cudaFree(d_cell_movements_);
}
