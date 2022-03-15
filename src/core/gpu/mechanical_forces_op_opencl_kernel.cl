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

real norm(real3);
real squared_euclidian_distance(__global real*, uint, uint);
int3 get_box_coordinates(uint, __constant uint*);
uint get_box_id(int3,__constant uint*);
void compute_force(__global real*, __global real*, uint, uint, real3*);
void force(__global real*, __global real*, uint, uint, ushort, __global uint*, real, real3*);

real norm(real3 v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

real squared_euclidian_distance(__global real* positions, uint idx, uint nidx) {
  const real dx = positions[3*idx + 0] - positions[3*nidx + 0];
  const real dy = positions[3*idx + 1] - positions[3*nidx + 1];
  const real dz = positions[3*idx + 2] - positions[3*nidx + 2];
  return (dx * dx + dy * dy + dz * dz);
}

int3 get_box_coordinates(uint box_idx, __constant uint* num_boxes_axis_) {
  int3 box_coord;
  box_coord.z = box_idx / (num_boxes_axis_[0]*num_boxes_axis_[1]);
  uint remainder = box_idx % (num_boxes_axis_[0]*num_boxes_axis_[1]);
  box_coord.y = remainder / num_boxes_axis_[0];
  box_coord.x = remainder % num_boxes_axis_[0];
  return box_coord;
}

uint get_box_id(int3 bc,__constant uint* num_boxes_axis) {
  return bc.z * num_boxes_axis[0]*num_boxes_axis[1] + bc.y * num_boxes_axis[0] + bc.x;
}

void compute_force(__global real* positions, __global real* diameters, uint idx, uint nidx, real3* collision_force) {
  real r1 = 0.5 * diameters[idx];
  real r2 = 0.5 * diameters[nidx];
  // We take virtual bigger radii to have a distant interaction, to get a desired density.
  real additional_radius = 10.0 * 0.15;
  r1 += additional_radius;
  r2 += additional_radius;

  real comp1 = positions[3*idx + 0] - positions[3*nidx + 0];
  real comp2 = positions[3*idx + 1] - positions[3*nidx + 1];
  real comp3 = positions[3*idx + 2] - positions[3*nidx + 2];
  real center_distance = sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);

  // the overlap distance (how much one penetrates in the other)
  real delta = r1 + r2 - center_distance;

  if (delta < 0) {
    return;
  }

  // to avoid a division by 0 if the centers are (almost) at the same location
  // TODO(ahmad): change this to random values
  if (center_distance < 0.00000001) {
    collision_force->x += 42.0;
    collision_force->y += 42.0;
    collision_force->z += 42.0;
    return;
  }

  // the force itself
  real r = (r1 * r2) / (r1 + r2);
  real gamma = 1; // attraction coeff
  real k = 2;     // repulsion coeff
  real f = k * delta - gamma * sqrt(r * delta);

  real module = f / center_distance;
  collision_force->x += module * comp1;
  collision_force->y += module * comp2;
  collision_force->z += module * comp3;
  // printf(\"collision_force = (%f, %f, %f)\\n\", collision_force->x, collision_force->y, collision_force->z);
}


void force(__global real* positions,
                   __global real* diameters,
                   uint idx, uint start, ushort length,
                   __global uint* successors, real squared_radius,
                   real3* collision_force) {
  uint nidx = start;

  for (ushort nb = 0; nb < length; nb++) {
    // implement logic for within radius here
    if (nidx != idx) {
      if (squared_euclidian_distance(positions, idx, nidx) < squared_radius) {
        compute_force(positions, diameters, idx, nidx, collision_force);
      }
    }
    // traverse linked-list
    nidx = successors[nidx];
  }
}

__kernel void collide(__global real* positions,
                      __global real* diameters,
                      __global real* tractor_force,
                      __global real* adherence,
                      __global uint* box_id,
                      __global real* mass,
                      real timestep,
                      real max_displacement,
                      real squared_radius,
                      uint N,
                      __global uint* starts,
                      __global ushort* lengths,
                      __global uint* successors,
                      __constant uint* num_boxes_axis,
                      __global real* result) {
  uint tidx = get_global_id(0);
  if (tidx < N) {
    // Apply tractor forces
    result[3*tidx + 0] = timestep * tractor_force[3*tidx + 0];
    result[3*tidx + 1] = timestep * tractor_force[3*tidx + 1];
    result[3*tidx + 2] = timestep * tractor_force[3*tidx + 2];
    // printf(\"cell_movement = (%f, %f, %f)\\n\", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);

    real3 collision_force = (real3)(0, 0, 0);

    // Moore neighborhood
    int3 box_coords = get_box_coordinates(box_id[tidx], num_boxes_axis);
    for (int z = -1; z <= 1; z++) {
      for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
          uint bidx = get_box_id(box_coords + (int3)(x, y, z), num_boxes_axis);
          if (lengths[bidx] != 0) {
            force(positions, diameters, tidx, starts[bidx], lengths[bidx], successors, squared_radius, &collision_force);
          }
        }
      }
    }

    // Mass needs to non-zero!
    real mh = timestep / mass[tidx];
    // printf(\"mh = %f\\n\", mh);

    if (norm(collision_force) > adherence[tidx]) {
      result[3*tidx + 0] += collision_force.x * mh;
      result[3*tidx + 1] += collision_force.y * mh;
      result[3*tidx + 2] += collision_force.z * mh;
      // printf(\"cell_movement (1) = (%f, %f, %f)\\n\", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);

      if (norm(collision_force) * mh > max_displacement) {
        result[3*tidx + 0] = max_displacement;
        result[3*tidx + 1] = max_displacement;
        result[3*tidx + 2] = max_displacement;
      }
    }
    // printf(\"cell_movement (2) = (%f, %f, %f)\\n\", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);
  }
}

__kernel void clear_force_opencl(__global real* result, uint N) {
  uint tidx = get_global_id(0);
  if (tidx < N * N * N) {
    result[3*tidx + 0] = 0;
    result[3*tidx + 1] = 0;
    result[3*tidx + 2] = 0;
  }
}
