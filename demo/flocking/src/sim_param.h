// -----------------------------------------------------------------------------
//
// Copyright (C) Moritz Grabmann.
// All Rights Reserved.
//
// -----------------------------------------------------------------------------

#ifndef SIM_PARAM_H_
#define SIM_PARAM_H_

#include "biodynamo.h"
#include "core/container/math_array.h"

namespace bdm {

// -----------------------------------------------------------------------------
// Parameters specific for this simulation
// -----------------------------------------------------------------------------
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);

  uint64_t computational_steps = 8000;
  size_t n_boids = 250;
  double starting_sphere_radius = 200;

  double actual_diameter = 10;
  double boid_perception_radius = 250;
  double boid_interaction_radius = 70;
  double perception_angle_deg = 300;
  double neighbor_distance = 50;
  double max_accel = 0.4;
  double max_speed = 5;
  bool limit_speed = true;

  // Flocking 2 Algorithm
  double c_a_1 = 0.37;
  double c_a_2 = 0.05;
  double c_a_3 = 0.05;
  double c_y = 0.05;
  double h_a = 0.25;
  double eps = 0.1;
  double d_t = 0.05;
  Double3 pos_gamma = {1000, 0, 0};
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
