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
  real_t starting_sphere_radius = 200;

  real_t actual_diameter = 10;
  real_t boid_perception_radius = 250;
  real_t boid_interaction_radius = 70;
  real_t perception_angle_deg = 300;
  real_t neighbor_distance = 50;
  real_t max_accel = 0.4;
  real_t max_speed = 5;
  real_t random_perturbation_strength = 0.5;
  bool limit_speed = true;

  // Flocking 2 Algorithm
  real_t c_a_1 = 0.37;
  real_t c_a_2 = 0.05;
  real_t c_a_3 = 0.05;
  real_t c_y = 0.05;
  real_t h_a = 0.25;
  real_t eps = 0.1;
  real_t d_t = 0.05;
  Real3 pos_gamma = {1000, 0, 0};

  // Fluctuation parameter
  real_t period_x_c_a_1 = 200;
  real_t period_x_c_a_2 = 400;
  real_t period_x_c_a_3 = 600;
  real_t period_x_c_y = 150;
  real_t period_y_c_a_1 = 200;
  real_t period_y_c_a_2 = 400;
  real_t period_y_c_a_3 = 600;
  real_t period_y_c_y = 150;
  real_t period_z_c_a_1 = 200;
  real_t period_z_c_a_2 = 400;
  real_t period_z_c_a_3 = 600;
  real_t period_z_c_y = 150;
  real_t period_t_c_a_1 = 1e20;
  real_t period_t_c_a_2 = 1e20;
  real_t period_t_c_a_3 = 1e20;
  real_t period_t_c_y = 1e20;
  real_t fluctuation_strength_c_a_1 = 0.9;
  real_t fluctuation_strength_c_a_2 = 0.9;
  real_t fluctuation_strength_c_a_3 = 0.9;
  real_t fluctuation_strength_c_y = 0.9;
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
