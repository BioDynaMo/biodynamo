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

#include "cell_cell_force.h"
#include "sim_param.h"

namespace bdm {

/// Custom force. Changed adhesive and repulsive parameters compared to standard
/// force to achieve quick separation of mother and daughter cells after
/// division.
Real4 CellCellForce::Calculate(const Agent* lhs, const Agent* rhs) const {
  const Real3& ref_mass_location = lhs->GetPosition();
  real_t ref_diameter = lhs->GetDiameter();
  real_t ref_iof_coefficient = 0.15;
  const Real3& nb_mass_location = rhs->GetPosition();
  real_t nb_diameter = rhs->GetDiameter();
  real_t nb_iof_coefficient = 0.15;

  auto c1 = ref_mass_location;
  real_t r1 = 0.5 * ref_diameter;
  auto c2 = nb_mass_location;
  real_t r2 = 0.5 * nb_diameter;
  // We take virtual bigger radii to have a distant interaction, to get a
  // desired density.
  real_t additional_radius =
      10.0 * std::min(ref_iof_coefficient, nb_iof_coefficient);
  r1 += additional_radius;
  r2 += additional_radius;
  // the 3 components of the vector c2 -> c1
  real_t comp1 = c1[0] - c2[0];
  real_t comp2 = c1[1] - c2[1];
  real_t comp3 = c1[2] - c2[2];
  real_t center_distance =
      std::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
  // the overlap distance (how much one penetrates in the other)
  real_t delta = r1 + r2 - center_distance;
  // if no overlap : no force
  if (delta < 0) {
    return {0.0, 0.0, 0.0, 0.0};
  }
  // to avoid a division by 0 if the centers are (almost) at the same
  //  location
  if (center_distance < 0.00000001) {
    auto* random = Simulation::GetActive()->GetRandom();
    auto force2on1 = random->template UniformArray<3>(-3.0, 3.0);
    return {force2on1[0], force2on1[1], force2on1[2], 0};
  }
  // the force itself
  const auto* sparam =
      Simulation::GetActive()
          ->GetParam()
          ->Get<SimParam>();  // get a pointer to an instance of SimParam
  real_t r = (r1 * r2) / (r1 + r2);
  real_t gamma = sparam->attraction_coeff;
  real_t k = sparam->repulsion_coeff;
  real_t f = k * delta - gamma * std::sqrt(r * delta);

  real_t module = f / center_distance;
  Real3 force2on1({module * comp1, module * comp2, module * comp3});
  return {force2on1[0], force2on1[1], force2on1[2], 0};
}

}  // namespace bdm
