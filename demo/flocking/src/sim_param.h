// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// Author: Moritz Grabmann (2022)
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
  real starting_sphere_radius = 200;

  real actual_diameter = 10;
  real boid_perception_radius = 250;
  real boid_interaction_radius = 70;
  real perception_angle_deg = 300;
  real neighbor_distance = 50;
  real max_accel = 0.4;
  real max_speed = 5;
  bool limit_speed = true;

  // Flocking 2 Algorithm
  real c_a_1 = 0.37;
  real c_a_2 = 0.05;
  real c_a_3 = 0.05;
  real c_y = 0.05;
  real h_a = 0.25;
  real eps = 0.1;
  real d_t = 0.05;
  Real3 pos_gamma = {1000, 0, 0};
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
