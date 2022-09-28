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

#ifndef SIM_PARAM_H_
#define SIM_PARAM_H_

#include "biodynamo.h"

namespace bdm {

// Parameters specific for this simulation
struct SimParam : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(SimParam, 1);
  real_t const cell_diam = 20.;
  real_t const nn = 1140;  // Monolayer initial size
  real_t const pos0 = -(nn / 2.) + (cell_diam / 2.);
  real_t const posN = nn / 2.;
  size_t const count_cell_freq = 5;
  size_t const ts_freq = 200;
  size_t const time_steps =
      3100;  // Run simulation for 310 hours (1 timestep = 0.1 hours)
  real_t const step_length = 0.1;  // h
  real_t const t0 =
      14.;  // Initial time (days) needed to compare against exp results
  real_t const attraction_coeff = 0.001;  // attraction coeff default 1
  real_t const repulsion_coeff = 30000;   // repulsion coeff default 2
  int const neighbours_threshold = 20;
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
