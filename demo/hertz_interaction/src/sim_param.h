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
  real_t cell_diam = 10.; // um

  // Cells initial positions
  Real3 cell1_position = {-4.5, 0., 0.};
  Real3 cell2_position = {4.5, 0., 0.};

  // Simulation time
  real_t total_time = 0.01;

  // Extended Hertz force parameters
  real_t poisson_ratio = 0.4;
  real_t young_modulus = 1e-9; // 1000 Pa -> 1 Pa = 1 N/m^2 = 1 N/10^12 um^2 = 10^-12 N/um^2 -> 1000 Pa = 10^-9 N/um^2
  real_t density_adhesion_molecules = 1e-7; // 1e5 1/m^2 -> 1e-7 1/um^2
  real_t binding_energy_single_bond = 8.28e-14; // 20 k_b T = 20 * 1.380649 × 10-23 m2 kg s-2 K-1 * 300 K = 8.28e-20 J = 8.28e-20 N*m = 8.28e-14 N*um

  real_t composite_young_modulus = (1 / ((1 - poisson_ratio * poisson_ratio) / young_modulus + (1 - poisson_ratio * poisson_ratio) / young_modulus));
  real_t specific_adhesion_energy = density_adhesion_molecules * binding_energy_single_bond;
};

}  // namespace bdm

#endif  // SIM_PARAM_H_
