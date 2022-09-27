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
// -----------------------------------------------------------------------------

#include "cell_cell_force.h"
#include "sim_param.h"

namespace bdm {

Double4 CellCellForce::Calculate(const Agent* lhs, const Agent* rhs) const {
  const Double3& ref_mass_location = lhs->GetPosition();
  double ref_diameter = lhs->GetDiameter();
  double ref_iof_coefficient = 0.15;
  const Double3& nb_mass_location = rhs->GetPosition();
  double nb_diameter = rhs->GetDiameter();
  double nb_iof_coefficient = 0.15;

  auto c1 = ref_mass_location;
  double r1 = 0.5 * ref_diameter;
  auto c2 = nb_mass_location;
  double r2 = 0.5 * nb_diameter;
  // We take virtual bigger radii to have a distant interaction, to get a
  // desired density.
  double additional_radius =
      10.0 * std::min(ref_iof_coefficient, nb_iof_coefficient);
  r1 += additional_radius;
  r2 += additional_radius;
  // the 3 components of the vector c2 -> c1
  double comp1 = c1[0] - c2[0];
  double comp2 = c1[1] - c2[1];
  double comp3 = c1[2] - c2[2];
  double center_distance =
      std::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
  // the overlap distance (how much one penetrates in the other)
  double delta = r1 + r2 - center_distance;
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
  auto* sparam =
      Simulation::GetActive()
          ->GetParam()
          ->Get<SimParam>();  // get a pointer to an instance of SimParam
  double r = (r1 * r2) / (r1 + r2);
  double gamma = sparam->attraction_coeff;
  double k = sparam->repulsion_coeff;
  double f = k * delta - gamma * std::sqrt(r * delta);

  double module = f / center_distance;
  Double3 force2on1({module * comp1, module * comp2, module * comp3});
  return {force2on1[0], force2on1[1], force2on1[2], 0};
}

}  // namespace bdm