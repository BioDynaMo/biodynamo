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

double norm(double3 v) {
  return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

double squared_euclidian_distance(__global double* positions, uint idx, uint nidx) {
  const double dx = positions[3*idx + 0] - positions[3*nidx + 0];
  const double dy = positions[3*idx + 1] - positions[3*nidx + 1];
  const double dz = positions[3*idx + 2] - positions[3*nidx + 2];
  return (dx * dx + dy * dy + dz * dz);
}

int3 get_box_coordinates(double3 pos, __constant int* grid_dimensions, uint box_length) {
  int3 box_coords;
  box_coords.x = (floor(pos.x) - grid_dimensions[0]) / box_length;
  box_coords.y = (floor(pos.y) - grid_dimensions[1]) / box_length;
  box_coords.z = (floor(pos.z) - grid_dimensions[2]) / box_length;
  return box_coords;
}

int3 get_box_coordinates_2(uint box_idx, __constant uint* num_boxes_axis_) {
  int3 box_coord;
  box_coord.z = box_idx / (num_boxes_axis_[0]*num_boxes_axis_[1]);
  uint remainder = box_idx % (num_boxes_axis_[0]*num_boxes_axis_[1]);
  box_coord.y = remainder / num_boxes_axis_[0];
  box_coord.x = remainder % num_boxes_axis_[0];
  return box_coord;
}

uint get_box_id_2(int3 bc,__constant uint* num_boxes_axis) {
  return bc.z * num_boxes_axis[0]*num_boxes_axis[1] + bc.y * num_boxes_axis[0] + bc.x;
}

uint get_box_id(double3 pos, __constant uint* num_boxes_axis, __constant int* grid_dimensions, uint box_length) {
  int3 box_coords = get_box_coordinates(pos, grid_dimensions, box_length);
  return get_box_id_2(box_coords, num_boxes_axis);
}

void compute_force(__global double* positions, __global double* diameters, uint idx, uint nidx, double3* collision_force) {
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
  // TODO(ahmad): change this to random values
  if (center_distance < 0.00000001) {
    collision_force->x += 42.0;
    collision_force->y += 42.0;
    collision_force->z += 42.0;
    return;
  }

  // the force itself
  double r = (r1 * r2) / (r1 + r2);
  double gamma = 1; // attraction coeff
  double k = 2;     // repulsion coeff
  double f = k * delta - gamma * sqrt(r * delta);

  double module = f / center_distance;
  collision_force->x += module * comp1;
  collision_force->y += module * comp2;
  collision_force->z += module * comp3;
  // printf(\"collision_force = (%f, %f, %f)\\n\", collision_force->x, collision_force->y, collision_force->z);
}


void default_force(__global double* positions,
                   __global double* diameters,
                   uint idx, uint start, ushort length,
                   __global uint* successors, double squared_radius,
                   double3* collision_force) {
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

__kernel void collide(__global double* positions,
                      __global double* diameters,
                      __global double* tractor_force,
                      __global double* adherence,
                      __global uint* box_id,
                      __global double* mass,
                      double timestep,
                      double max_displacement,
                      double squared_radius,
                      uint N,
                      __global uint* starts,
                      __global ushort* lengths,
                      __global uint* successors,
                      uint box_length,
                      __constant uint* num_boxes_axis,
                      __constant int* grid_dimensions,
                      __global double* result) {
  uint tidx = get_global_id(0);
  if (tidx < N) {
    // Apply tractor forces
    result[3*tidx + 0] = timestep * tractor_force[3*tidx + 0];
    result[3*tidx + 1] = timestep * tractor_force[3*tidx + 1];
    result[3*tidx + 2] = timestep * tractor_force[3*tidx + 2];
    // printf(\"cell_movement = (%f, %f, %f)\\n\", result[3*tidx + 0], result[3*tidx + 1], result[3*tidx + 2]);

    double3 collision_force = (double3)(0, 0, 0);

    // Moore neighborhood
    int3 box_coords = get_box_coordinates_2(box_id[tidx], num_boxes_axis);
    for (int z = -1; z <= 1; z++) {
      for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
          uint bidx = get_box_id_2(box_coords + (int3)(x, y, z), num_boxes_axis);
          if (lengths[bidx] != 0) {
            default_force(positions, diameters, tidx, starts[bidx], lengths[bidx], successors, squared_radius, &collision_force);
          }
        }
      }
    }

    // Mass needs to non-zero!
    double mh = timestep / mass[tidx];
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

__kernel void clear_force_opencl(__global double* result, uint N) {
  uint tidx = get_global_id(0);
  if (tidx < N * N * N) {
    result[3*tidx + 0] = 0;
    result[3*tidx + 1] = 0;
    result[3*tidx + 2] = 0;
  }
}
