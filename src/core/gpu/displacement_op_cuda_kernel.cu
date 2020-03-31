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

#include "core/gpu/displacement_op_cuda_kernel.h"
#include "core/gpu/math.h"
#include "core/shape.h"

#include "samples/common/inc/helper_math.h"

namespace bdm {

#define GpuErrchk(ans) \
  { GpuAssert((ans), __FILE__, __LINE__); }
inline void GpuAssert(cudaError_t code, const char* file, int line,
                      bool abort = true) {
  if (code != cudaSuccess) {
    fprintf(stderr, "GPUassert (error code %d): %s %s %d\n", code,
            cudaGetErrorString(code), file, line);
    if (code == cudaErrorInsufficientDriver) {
      printf(
          "This probably means that no CUDA-compatible GPU has been detected. "
          "Consider setting the use_opencl flag to \"true\" in the bmd.toml "
          "file to use OpenCL instead.\n");
    }
    if (abort)
      exit(code);
  }
}

inline __device__ double GetL2Distance(const double3& a, const double3& b) {
  double3 dist_array = b - a;
  return Norm(dist_array);
}

inline __device__ double Sum(const double3& a) { return a.x + a.y + a.z; }

__device__ double SquaredEuclidianDistance(const double* positions,
                                           uint32_t idx, uint32_t nidx) {
  const double dx = positions[3 * idx + 0] - positions[3 * nidx + 0];
  const double dy = positions[3 * idx + 1] - positions[3 * nidx + 1];
  const double dz = positions[3 * idx + 2] - positions[3 * nidx + 2];
  return (dx * dx + dy * dy + dz * dz);
}

__device__ int3 GetBoxCoordinates(uint32_t box_idx, uint32_t* num_boxes_axis) {
  int3 box_coord;
  box_coord.z = box_idx / (num_boxes_axis[0] * num_boxes_axis[1]);
  uint32_t remainder = box_idx % (num_boxes_axis[0] * num_boxes_axis[1]);
  box_coord.y = remainder / num_boxes_axis[0];
  box_coord.x = remainder % num_boxes_axis[0];
  return box_coord;
}

__device__ uint32_t GetBoxId(int3 bc, uint32_t* num_boxes_axis) {
  return bc.z * num_boxes_axis[0] * num_boxes_axis[1] +
         bc.y * num_boxes_axis[0] + bc.x;
}

__device__ void ForceBetweenSpheres(const double* positions,
                                    const double* diameters, uint32_t idx,
                                    uint32_t nidx, double3* result) {
  double r1 = 0.5 * diameters[idx];
  double r2 = 0.5 * diameters[nidx];
  // We take virtual bigger radii to have a distant interaction, to get a
  // desired density.
  double additional_radius = 10.0 * 0.15;
  r1 += additional_radius;
  r2 += additional_radius;

  double3 p1 = make_double3(positions, idx);
  double3 p2 = make_double3(positions, nidx);
  double3 comp = p1 - p2;
  double center_distance = Norm(comp);

  // the overlap distance (how much one penetrates in the other)
  double delta = r1 + r2 - center_distance;

  if (delta < 0) {
    return;
  }

  // to avoid a division by 0 if the centers are (almost) at the same location
  if (center_distance < 0.00000001) {
    *result += make_double3(42, 42, 42);
    return;
  }

  // the force itself
  double r = (r1 * r2) / (r1 + r2);
  double gamma = 1;  // attraction coeff
  double k = 2;      // repulsion coeff
  double f = k * delta - gamma * sqrt(r * delta);

  double module = f / center_distance;
  *result += comp * module;
}

__device__ double3 ComputeForceOfASphereOnASphere(const double3& c1, double r1,
                                                  const double3& c2,
                                                  double r2) {
  double3 comp = c1 - c2;
  double distance_between_centers = Norm(comp);
  double a = r1 + r2 - distance_between_centers;
  if (a < 0) {
    return make_double3(0.0, 0.0, 0.0);
  }
  if (distance_between_centers < 0.00000001) {
    return make_double3(42, 42, 42);
  } else {
    double module = a / distance_between_centers;
    return comp * module;
  }
}

__device__ void ForceOnACylinderFromASphere(
    const double* positions, const double* diameters,
    const double* ne_proximal_end, const double* ne_distal_end,
    const double* ne_axis, uint32_t idx, uint32_t nidx, double3* result,
    double* proportion_to_proximal_end) {
  double3 proximal_end = make_double3(ne_proximal_end, idx);
  double3 distal_end = make_double3(ne_distal_end, idx);
  double3 axis = make_double3(ne_axis, idx);

  double actual_length = Norm(axis);
  double d = diameters[idx];
  double3 c = make_double3(positions, nidx);
  double r = 0.5 * diameters[nidx];

  if (actual_length < r) {
    double rc = 0.5 * d;
    double3 dvec = (axis / actual_length) * rc;
    double3 npd = distal_end - dvec;
    *result = ComputeForceOfASphereOnASphere(npd, rc, c, r);
    return;
  }

  double3 proximal_end_closest = c - proximal_end;

  double proximal_end_closest_axis = Sum(proximal_end_closest * axis);
  double k = proximal_end_closest_axis / (actual_length * actual_length);
  double3 cc = proximal_end + (axis * k);

  if (k <= 1.0 && k >= 0.0) {
    *proportion_to_proximal_end = 1.0 - k;
  } else if (k < 0) {
    *proportion_to_proximal_end = 1.0;
    cc = proximal_end;
  } else {
    *proportion_to_proximal_end = 0.0;
    cc = distal_end;
  }

  double penetration = d / 2 + r - GetL2Distance(c, cc);
  if (penetration <= 0) {
    *result = make_double3(0.0, 0.0, 0.0);
    return;
  }
  *result = ComputeForceOfASphereOnASphere(cc, d * 0.5, c, r);
  return;
}

__device__ void ForceOnASphereFromACylinder(
    const double* positions, const double* diameters,
    const double* ne_proximal_end, const double* ne_distal_end,
    const double* ne_axis, uint32_t idx, uint32_t nidx, double3* result,
    double* proportion_to_proximal_end) {
  ForceOnACylinderFromASphere(positions, diameters, ne_proximal_end,
                              ne_distal_end, ne_axis, nidx, idx, result,
                              proportion_to_proximal_end);
}

__device__ void ForceBetweenCylinders(const double* positions,
                                      const double* diameters,
                                      const double* ne_proximal_end,
                                      uint32_t idx, uint32_t nidx,
                                      double3* result,
                                      double* proportion_to_proximal_end) {
  double3 a = make_double3(ne_proximal_end, idx);
  double3 c = make_double3(ne_proximal_end, nidx);
  double3 b = make_double3(positions, idx);
  double3 d = make_double3(positions, nidx);

  double d1 = diameters[idx];
  double d2 = diameters[nidx];

  double k = 0.5;  // part devoted to the distal node

  double p13x = a.x - c.x;
  double p13y = a.y - c.y;
  double p13z = a.z - c.z;
  double p43x = d.x - c.x;
  double p43y = d.y - c.y;
  double p43z = d.z - c.z;
  double p21x = b.x - a.x;
  double p21y = b.y - a.y;
  double p21z = b.z - a.z;

  double d1343 = p13x * p43x + p13y * p43y + p13z * p43z;
  double d4321 = p21x * p43x + p21y * p43y + p21z * p43z;
  double d1321 = p21x * p13x + p21y * p13y + p21z * p13z;
  double d4343 = p43x * p43x + p43y * p43y + p43z * p43z;
  double d2121 = p21x * p21x + p21y * p21y + p21z * p21z;

  double3 p1, p2;
  double denom = d2121 * d4343 - d4321 * d4321;

  // if the two segments are not ABSOLUTLY parallel
  if (denom > 0.000000000001) {
    double numer = d1343 * d4321 - d1321 * d4343;

    double mua = numer / denom;
    double mub = (d1343 + mua * d4321) / d4343;

    if (mua < 0) {
      p1 = a;
      k = 1;
    } else if (mua > 1) {
      p1 = b;
      k = 0;
    } else {
      p1 = make_double3(a.x + mua * p21x, a.y + mua * p21y, a.z + mua * p21z);
      k = 1 - mua;
    }

    if (mub < 0) {
      p2 = c;
    } else if (mub > 1) {
      p2 = d;
    } else {
      p2 = make_double3(c.x + mub * p43x, c.y + mub * p43y, c.z + mub * p43z);
    }

  } else {
    p1 = a + (b - a) * 0.5;
    p2 = c + (d - c) * 0.5;
  }

  // W put a virtual sphere on the two cylinders
  *result = ComputeForceOfASphereOnASphere(p1, d1 / 2.0, p2, d2 / 2.0) * 10;
  *proportion_to_proximal_end = k;
}

__device__ void GetForce(uint32_t idx, uint32_t nidx, const uint8_t* shape,
                         const double* positions, const double* diameters,
                         const double* ne_proximal_end,
                         const double* ne_distal_end, const double* ne_axis,
                         const uint32_t* daughter_left,
                         const uint32_t* daughter_right, const uint32_t* mother,
                         double3* result, double* ppe = nullptr) {
  if (shape[idx] == Shape::kSphere && shape[nidx] == Shape::kSphere) {
    ForceBetweenSpheres(positions, diameters, idx, nidx, result);
  } else if (shape[idx] == Shape::kSphere && shape[nidx] == Shape::kCylinder) {
    ForceOnASphereFromACylinder(positions, diameters, ne_proximal_end,
                                ne_distal_end, ne_axis, idx, nidx, result, ppe);
  } else if (shape[idx] == Shape::kCylinder && shape[nidx] == Shape::kSphere) {
    ForceOnACylinderFromASphere(positions, diameters, ne_proximal_end,
                                ne_distal_end, ne_axis, idx, nidx, result, ppe);
  } else if (shape[idx] == Shape::kCylinder &&
             shape[nidx] == Shape::kCylinder) {
    ForceBetweenCylinders(positions, diameters, ne_proximal_end, idx, nidx,
                          result, ppe);
  }
}

__device__ void DisplacementCylinder(
    uint32_t idx, const uint8_t* shape, const double* positions,
    const double* diameters, const double* ne_proximal_end,
    const double* ne_distal_end, const double* ne_axis,
    const uint32_t* daughter_left, const uint32_t* daughter_right,
    const uint32_t* mother, uint32_t start, uint16_t length,
    const uint32_t* successors, const double* squared_radius, double3* result,
    double3* result2, bool* has_neurite_neighbor) {
  uint32_t nidx = start;
  for (uint16_t nb = 0; nb < length; nb++) {
    if (nidx != idx) {
      if (SquaredEuclidianDistance(positions, idx, nidx) < *squared_radius) {
        // TODO: we should probably also check if there is a daughter or
        // mother with `has_daughter_or_mother`
        if (shape[nidx] == Shape::kCylinder) {
          if (daughter_left[idx] == nidx || daughter_right[idx] == nidx ||
              mother[idx] == nidx || mother[idx] == mother[nidx]) {
            return;
          }
        }
        double3 force_from_neighbor = make_double3(0, 0, 0);
        double proportion_to_proximal_end = 0;
        GetForce(idx, nidx, shape, positions, diameters, ne_proximal_end,
                 ne_distal_end, ne_axis, daughter_left, daughter_right, mother,
                 &force_from_neighbor, &proportion_to_proximal_end);

        double h_over_m = 0.01;
        if (shape[nidx] == Shape::kCylinder) {
          force_from_neighbor = force_from_neighbor * h_over_m;
          *has_neurite_neighbor = true;
        }

        if (proportion_to_proximal_end < 1E-10) {
          *result += force_from_neighbor;
        } else {
          double part_for_point_mass = 1.0 - proportion_to_proximal_end;
          *result += force_from_neighbor * part_for_point_mass;
          *result2 += force_from_neighbor * proportion_to_proximal_end;
        }
      }
    }
    // traverse linked-list
    nidx = successors[nidx];
  }
}

__device__ void DisplacementSphere(
    uint32_t idx, const uint8_t* shape, const double* positions,
    const double* diameters, const double* ne_proximal_end,
    const double* ne_distal_end, const double* ne_axis,
    const uint32_t* daughter_left, const uint32_t* daughter_right,
    const uint32_t* mother, uint32_t start, uint16_t length,
    const uint32_t* successors, const double* squared_radius, double3* result) {
  uint32_t nidx = start;
  for (uint16_t nb = 0; nb < length; nb++) {
    if (nidx != idx) {
      if (SquaredEuclidianDistance(positions, idx, nidx) < squared_radius[0]) {
        GetForce(idx, nidx, shape, positions, diameters, ne_proximal_end,
                 ne_distal_end, ne_axis, daughter_left, daughter_right, mother,
                 result);
      }
    }
  }
  // traverse linked-list
  nidx = successors[nidx];
}

/// Entry point to resolving collisions between all objects on GPU
__global__ void ResolveCollisions(
    const uint8_t* shape, const double* positions, const double* diameters,
    const double* tractor_force, const double* adherence,
    const uint32_t* box_id, const double* mass, const double* ne_proximal_end,
    const double* ne_distal_end, const double* ne_axis,
    const double* ne_tension, const double* force_to_transmit_to_proximal_mass,
    const uint32_t* daughter_left, const uint32_t* daughter_right,
    const uint32_t* mother, const uint8_t* has_daughter_or_mother,
    const double* timestep, const double* max_displacement,
    const double* squared_radius, const uint32_t* num_objects, uint32_t* starts,
    uint16_t* lengths, uint64_t* timestamps, uint64_t* current_timestamp,
    uint32_t* successors, uint32_t* box_length, uint32_t* num_boxes_axis,
    int32_t* grid_dimensions, double* result, double* result2) {
  uint32_t tidx = blockIdx.x * blockDim.x + threadIdx.x;
  if (tidx < *num_objects) {
    double3 movement_at_next_step = make_double3(0, 0, 0);
    if (shape[tidx] == Shape::kSphere) {  // If shape is spherical
      double3 translation_force_on_point_mass = make_double3(0, 0, 0);
      double3 tf =
          make_double3(tractor_force[3 * tidx + 0], tractor_force[3 * tidx + 1],
                       tractor_force[3 * tidx + 2]);

      movement_at_next_step = tf * (*timestep);

      // Moore neighborhood
      int3 box_coords = GetBoxCoordinates(box_id[tidx], num_boxes_axis);
      for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
          for (int x = -1; x <= 1; x++) {
            uint32_t bidx =
                GetBoxId(box_coords + make_int3(x, y, z), num_boxes_axis);
            if (timestamps[bidx] == *current_timestamp && lengths[bidx] != 0) {
              DisplacementSphere(
                  tidx, shape, positions, diameters, ne_proximal_end,
                  ne_distal_end, ne_axis, daughter_left, daughter_right, mother,
                  starts[bidx], lengths[bidx], successors, squared_radius,
                  &translation_force_on_point_mass);
            }
          }
        }
      }

      // Mass needs to non-zero!
      double mh = *timestep / mass[tidx];

      if (Norm(translation_force_on_point_mass) > adherence[tidx]) {
        movement_at_next_step += translation_force_on_point_mass * mh;

        if (Norm(translation_force_on_point_mass) * mh > *max_displacement) {
          movement_at_next_step = Normalize(movement_at_next_step);
          movement_at_next_step *= *max_displacement;
        }
      }
    } else {  // If shape is cylindrical
      double3 force_on_my_point_mass = make_double3(0, 0, 0);
      double3 force_on_my_mothers_point_mass = make_double3(0, 0, 0);
      double3 force_from_neighbors = make_double3(0, 0, 0);

      double3 axis = make_double3(ne_axis, tidx);
      double factor = -ne_tension[tidx] / Norm(axis);

      force_on_my_point_mass += axis * factor;

      if (has_daughter_or_mother[tidx] | kHasDaughterLeft == kHasDaughterLeft) {
        auto didx = daughter_left[tidx];
        double3 d_axis = make_double3(ne_axis, didx);
        auto f = ne_tension[didx] / Norm(d_axis);
        if (f < 0) {
          f = 0;
        }
        auto force_from_daughter =
            d_axis * f + make_double3(force_to_transmit_to_proximal_mass, didx);
        force_on_my_point_mass += force_from_daughter;
      }
      if (has_daughter_or_mother[tidx] |
          kHasDaughterRight == kHasDaughterRight) {
        auto didx = daughter_right[tidx];
        double3 d_axis = make_double3(ne_axis, didx);
        auto f = ne_tension[didx] / Norm(d_axis);
        if (f < 0) {
          f = 0;
        }
        auto force_from_daughter =
            d_axis * f + make_double3(force_to_transmit_to_proximal_mass, didx);
        force_on_my_point_mass += force_from_daughter;
      }

      double h_over_m = 0.01;
      bool has_neurite_neighbor = false;

      // Moore neighborhood
      int3 box_coords = GetBoxCoordinates(box_id[tidx], num_boxes_axis);
      for (int z = -1; z <= 1; z++) {
        for (int y = -1; y <= 1; y++) {
          for (int x = -1; x <= 1; x++) {
            uint32_t bidx =
                GetBoxId(box_coords + make_int3(x, y, z), num_boxes_axis);
            if (timestamps[bidx] == *current_timestamp && lengths[bidx] != 0) {
              DisplacementCylinder(
                  tidx, shape, positions, diameters, ne_proximal_end,
                  ne_distal_end, ne_axis, daughter_left, daughter_right, mother,
                  starts[bidx], lengths[bidx], successors, squared_radius,
                  &force_from_neighbors, &force_on_my_mothers_point_mass,
                  &has_neurite_neighbor);
            }
          }
        }
      }

      if (has_neurite_neighbor) {
        force_on_my_point_mass *= h_over_m;
      }

      force_on_my_point_mass += force_from_neighbors;
      result2[3 * tidx + 0] = force_on_my_mothers_point_mass.x;
      result2[3 * tidx + 1] = force_on_my_mothers_point_mass.y;
      result2[3 * tidx + 2] = force_on_my_mothers_point_mass.z;
      double force_norm = Norm(force_on_my_point_mass);
      if (force_norm > adherence[tidx]) {
        movement_at_next_step = force_on_my_point_mass;
        if (force_norm > *max_displacement) {
          movement_at_next_step *= *max_displacement / force_norm;
        }
      }
    }
    result[3 * tidx + 0] = movement_at_next_step.x;
    result[3 * tidx + 1] = movement_at_next_step.y;
    result[3 * tidx + 2] = movement_at_next_step.z;
  }
}

bdm::DisplacementOpCudaKernel::DisplacementOpCudaKernel(uint32_t num_objects,
                                                        uint32_t num_boxes) {
  GpuErrchk(cudaMalloc(&d_positions_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_shape_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_diameters_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_tractor_force_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_adherence_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_ne_proximal_end_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_ne_distal_end_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_ne_axis_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_ne_tension_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_ne_force_to_transmit_to_proximal_mass_,
                       3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_daughter_left_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_daughter_right_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_mother_, num_objects * sizeof(uint32_t)));
  GpuErrchk(
      cudaMalloc(&d_has_daughter_or_mother_, num_objects * sizeof(uint8_t)));
  GpuErrchk(cudaMalloc(&d_box_id_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_mass_, num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_timestep_, sizeof(double)));
  GpuErrchk(cudaMalloc(&d_max_displacement_, sizeof(double)));
  GpuErrchk(cudaMalloc(&d_squared_radius_, sizeof(double)));
  GpuErrchk(cudaMalloc(&d_num_objects_, sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_starts_, num_boxes * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_lengths_, num_boxes * sizeof(uint16_t)));
  GpuErrchk(cudaMalloc(&d_timestamps_, num_boxes * sizeof(uint64_t)));
  GpuErrchk(cudaMalloc(&d_current_timestamp_, sizeof(uint64_t)));
  GpuErrchk(cudaMalloc(&d_successors_, num_objects * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_box_length_, sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_num_boxes_axis_, 3 * sizeof(uint32_t)));
  GpuErrchk(cudaMalloc(&d_grid_dimensions_, 3 * sizeof(int32_t)));
  GpuErrchk(cudaMalloc(&d_cell_movements_, 3 * num_objects * sizeof(double)));
  GpuErrchk(cudaMalloc(&d_force_to_transmit_to_proximal_mass_,
                       3 * num_objects * sizeof(double)));
}

void bdm::DisplacementOpCudaKernel::LaunchDisplacementKernel(
    const uint8_t* shape, const double* positions, const double* diameters,
    const double* tractor_force, const double* adherence,
    const uint32_t* box_id, const double* mass, const double* ne_proximal_end,
    const double* ne_distal_end, const double* ne_axis,
    const double* ne_tension,
    const double* ne_force_to_transmit_to_proximal_mass,
    const uint32_t* daughter_left, const uint32_t* daughter_right,
    const uint32_t* mother, const uint8_t* has_daughter_or_mother,
    const double* timestep, const double* max_displacement,
    const double* squared_radius, const uint32_t* num_objects, uint32_t* starts,
    uint16_t* lengths, uint64_t* timestamps, uint64_t* current_timestamp,
    uint32_t* successors, uint32_t* box_length, uint32_t* num_boxes_axis,
    int32_t* grid_dimensions, double* cell_movements,
    double* force_to_transmit_to_proximal_mass) {
  uint32_t num_boxes =
      num_boxes_axis[0] * num_boxes_axis[1] * num_boxes_axis[2];

  GpuErrchk(cudaMemcpy(d_shape_, shape,
        num_objects[0] * sizeof(uint8_t),
        cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_positions_, positions,
                       3 * num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_diameters_, diameters, num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_tractor_force_, tractor_force,
                       3 * num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_adherence_, adherence, num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_ne_proximal_end_, ne_proximal_end,
                       3 * num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_ne_distal_end_, ne_distal_end,
                       3 * num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_ne_axis_, ne_axis, 3 * num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_ne_tension_, ne_tension,
                       num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_ne_force_to_transmit_to_proximal_mass_,
                       ne_force_to_transmit_to_proximal_mass,
                       3 * num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_daughter_left_, daughter_left,
                       num_objects[0] * sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_daughter_right_, daughter_right,
                       num_objects[0] * sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_mother_, mother, num_objects[0] * sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_has_daughter_or_mother_, has_daughter_or_mother,
                       num_objects[0] * sizeof(uint8_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_box_id_, box_id, num_objects[0] * sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_mass_, mass, num_objects[0] * sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_timestep_, timestep, sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_max_displacement_, max_displacement, sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_squared_radius_, squared_radius, sizeof(double),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_num_objects_, num_objects, sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_starts_, starts, num_boxes * sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_lengths_, lengths, num_boxes * sizeof(uint16_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_timestamps_, timestamps, num_boxes * sizeof(uint64_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_current_timestamp_, current_timestamp,
                       sizeof(uint64_t), cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_successors_, successors,
                       num_objects[0] * sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_box_length_, box_length, sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_num_boxes_axis_, num_boxes_axis, 3 * sizeof(uint32_t),
                       cudaMemcpyHostToDevice));
  GpuErrchk(cudaMemcpy(d_grid_dimensions_, grid_dimensions,
                       3 * sizeof(uint32_t), cudaMemcpyHostToDevice));

  int blockSize = 128;
  int minGridSize;
  int gridSize;

  // Get a near-optimal occupancy with the following thread organization
  cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize,
                                     ResolveCollisions, 0, num_objects[0]);
  gridSize = (num_objects[0] + blockSize - 1) / blockSize;

  // printf("gridSize = %d  |  blockSize = %d\n", gridSize, blockSize);
  ResolveCollisions<<<gridSize, blockSize>>>(
      d_shape_, d_positions_, d_diameters_, d_tractor_force_, d_adherence_,
      d_box_id_, d_mass_, d_ne_proximal_end_, d_ne_distal_end_, d_ne_axis_,
      d_ne_tension_, d_ne_force_to_transmit_to_proximal_mass_, d_daughter_left_,
      d_daughter_right_, d_mother_, d_has_daughter_or_mother_, d_timestep_,
      d_max_displacement_, d_squared_radius_, d_num_objects_, d_starts_,
      d_lengths_, d_timestamps_, d_current_timestamp_, d_successors_,
      d_box_length_, d_num_boxes_axis_, d_grid_dimensions_, d_cell_movements_,
      d_force_to_transmit_to_proximal_mass_);

  // We need to wait for the kernel to finish before reading back the result
  cudaDeviceSynchronize();
  cudaMemcpy(cell_movements, d_cell_movements_,
             3 * num_objects[0] * sizeof(double), cudaMemcpyDeviceToHost);
}

void bdm::DisplacementOpCudaKernel::ResizeCellBuffers(uint32_t num_cells) {
  cudaFree(d_shape_);
  cudaFree(d_positions_);
  cudaFree(d_diameters_);
  cudaFree(d_tractor_force_);
  cudaFree(d_adherence_);
  cudaFree(d_ne_proximal_end_);
  cudaFree(d_ne_distal_end_);
  cudaFree(d_ne_axis_);
  cudaFree(d_ne_tension_);
  cudaFree(d_ne_force_to_transmit_to_proximal_mass_);
  cudaFree(d_daughter_left_);
  cudaFree(d_daughter_right_);
  cudaFree(d_mother_);
  cudaFree(d_has_daughter_or_mother_);
  cudaFree(d_box_id_);
  cudaFree(d_mass_);
  cudaFree(d_successors_);
  cudaFree(d_cell_movements_);
  cudaFree(d_force_to_transmit_to_proximal_mass_);

  cudaMalloc(&d_shape_, num_cells * sizeof(uint8_t));
  cudaMalloc(&d_positions_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_diameters_, num_cells * sizeof(double));
  cudaMalloc(&d_tractor_force_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_ne_proximal_end_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_ne_distal_end_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_ne_axis_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_ne_tension_, num_cells * sizeof(double));
  cudaMalloc(&d_ne_force_to_transmit_to_proximal_mass_,
             3 * num_cells * sizeof(double));
  cudaMalloc(&d_adherence_, num_cells * sizeof(double));
  cudaMalloc(&d_daughter_left_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_daughter_right_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_mother_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_has_daughter_or_mother_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_box_id_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_mass_, num_cells * sizeof(double));
  cudaMalloc(&d_successors_, num_cells * sizeof(uint32_t));
  cudaMalloc(&d_cell_movements_, 3 * num_cells * sizeof(double));
  cudaMalloc(&d_force_to_transmit_to_proximal_mass_,
             3 * num_cells * sizeof(double));
}

void bdm::DisplacementOpCudaKernel::ResizeGridBuffers(uint32_t num_boxes) {
  cudaFree(d_starts_);
  cudaFree(d_lengths_);
  cudaFree(d_timestamps_);

  cudaMalloc(&d_starts_, num_boxes * sizeof(uint32_t));
  cudaMalloc(&d_lengths_, num_boxes * sizeof(uint16_t));
  cudaMalloc(&d_timestamps_, num_boxes * sizeof(uint64_t));
}

bdm::DisplacementOpCudaKernel::~DisplacementOpCudaKernel() {
  cudaFree(d_shape_);
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
  cudaFree(d_timestamps_);
  cudaFree(d_current_timestamp_);
  cudaFree(d_successors_);
  cudaFree(d_num_boxes_axis_);
  cudaFree(d_grid_dimensions_);
  cudaFree(d_cell_movements_);
  cudaFree(d_ne_proximal_end_);
  cudaFree(d_ne_distal_end_);
  cudaFree(d_ne_axis_);
  cudaFree(d_ne_tension_);
  cudaFree(d_ne_force_to_transmit_to_proximal_mass_);
  cudaFree(d_force_to_transmit_to_proximal_mass_);
  cudaFree(d_daughter_left_);
  cudaFree(d_daughter_right_);
  cudaFree(d_mother_);
  cudaFree(d_has_daughter_or_mother_);
}

}  // namespace bdm
