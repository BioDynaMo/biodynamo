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
  // Cell diameter
  real_t cell_diam = 20.;

  // Monolayer initial size
  real_t nn = 1140;

  // Positions for initialization of cells
  real_t pos0 = -(nn / 2.) + (cell_diam / 2.);
  real_t posN = nn / 2.;

  // Frequency of time series information extraction
  size_t ts_freq = 200;

  // Total simulation time in hours
  size_t total_time = 310;

  // Initial time in days (for plotting purposes)
  real_t t0 = 14.;

  // Forces: attraction coeff default 1
  real_t attraction_coeff = 0.001;

  // Forces: repulsion coeff default 2
  real_t repulsion_coeff = 300;

  // Threshold for cell division depending on how "filled" the surrounding is
  int neighbours_threshold = 20;
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
