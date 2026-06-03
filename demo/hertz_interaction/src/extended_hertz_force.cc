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

#include "extended_hertz_force.h"
#include "sim_param.h"

namespace bdm {

// Cell-cell interaction
// The interaction forces between cells are approximated either by an extended
// Hertz model (Galle et. al., Biophys. J. 2005) model, adding an adhesion term
// to the classical Hertz model that describes a homogeneous, isotropic elastic
// sphere upon small deformation using the same interface area as in the
// classical Hertz model (that has no adhesion), or the Johnson-Kendall-Roberts
// (JKR)-model, that takes into account the extra deformation of the contact
// area between two interacting spheres due to cell-cell adhesion. The JKR model
// has been validated for certain situation in cell-pipette experiments (Chu et.
// al., Phys. Rev. Lett, 94, 028102, 2005). The extended Hertz model may
// correspond to the Derjaguin–Muller–Toporov (DMT) model.

// Hertz-model.
// The extended Hertz-force

// F_ij^DMT=|▁F_ij^eHertz (d_ij)|

// where dij is the distance between the centers of two interacting spheres i
// and j that is calculated as

// F_ij^eHertz=(4E ̃_ij)/3 √(R ̃ ) δ^(3/2)-πγ ̃R ̃

// The effective radius R ̃ is defined by R ̃^(-1)=R_i^(-1)+R_j^(-1), where Ri is
// the radius of cell i. δ=δi + δj is the sum of the deformations of each cell
// (upon compression it is the overlap of the two spheres) along the axis
// linking the centers of these cells, whereby δ = Ri+Rj-dij dij is the distance
// between the centers of model cell i and cell j. E ̃_ij is the composite Young
// modulus defined by E ̃_ij^(-1)=(1-ν_i^2)E_i^(-1)+(1-ν_j^2)E_j^(-1). We
// approximate the specific adhesion energy by γ ̂≈ς_m W_s. Here, ς_m≈10^5/m^2 is
// the density of adhesion molecules, W_s≈15-25k_b T the binding energy of a
// single bond. The adhesion term neglects additional deformation of the contact
// area as a consequence of cell-cell adhesion, and directly uses the Hertz
// contact area. Hence, the force value is fully determined by measurable
// quantities with no free fit parameter. The force vector denoting the force of
// cell j on cell i results from ▁F_ij=F_ij^eHertz (d_ij)(▁r_i-▁r_j)/(|▁r_i-▁r_j
// |)

Real4 ExtendedHertzForce::Calculate(const Agent* lhs, const Agent* rhs) const {
  const auto* sparam =
      Simulation::GetActive()
          ->GetParam()
          ->Get<SimParam>();  // get a pointer to an instance of SimParam

  const Real3& ref_mass_location = lhs->GetPosition();
  real_t ref_diameter = lhs->GetDiameter();
  const Real3& nb_mass_location = rhs->GetPosition();
  real_t nb_diameter = rhs->GetDiameter();

  real_t r1 = 0.5 * ref_diameter;
  real_t r2 = 0.5 * nb_diameter;

  real_t r_eff = (r1 * r2) / (r1 + r2);  // effective radius

  auto c1 = ref_mass_location;
  auto c2 = nb_mass_location;

  // the 3 components of the vector c2 -> c1
  real_t comp1 = c1[0] - c2[0];
  real_t comp2 = c1[1] - c2[1];
  real_t comp3 = c1[2] - c2[2];
  real_t center_distance =
      std::sqrt(comp1 * comp1 + comp2 * comp2 + comp3 * comp3);
  // the overlap distance (how much one penetrates in the other)
  real_t delta = r1 + r2 - center_distance;
  // if no overlap : no force
  real_t epsilon = 1e-10;  // to avoid numerical issues with very small overlaps
  if ((delta + epsilon) < 0) {
    std::cout << "No force, no overlap. Distance between centers: "
              << center_distance << std::endl;
    return {0.0, 0.0, 0.0, 0.0};
  }
  // to avoid a division by 0 if the centers are (almost) at the same
  //  location
  if (center_distance < epsilon) {
    auto* random = Simulation::GetActive()->GetRandom();
    auto force2on1 = random->template UniformArray<3>(-3.0, 3.0);

    std::cout << "No force, centers are at the same location. Force is random: "
              << force2on1 << std::endl;
    return {force2on1[0], force2on1[1], force2on1[2], 0};
  }
  // the force itself

  real_t f = (4 / 3) * (sparam->composite_young_modulus * std::sqrt(r_eff) *
                        std::pow(delta, 1.5)) -
             M_PI * sparam->specific_adhesion_energy * r_eff;  // in N

  // std::cout << "Repulsive force: " << (4 * sparam->composite_young_modulus *
  // std::sqrt(r_eff) * std::pow(delta, 1.5)) / 3 << " N, Adhesive force: " <<
  // M_PI * sparam->specific_adhesion_energy * r_eff << " N, Total force: " << f
  // << " N" << std::endl;

  real_t module = f / center_distance;

  // std::cout << "Distance between centers: " << center_distance << " um,
  // Overlap: " << delta << " um, Force module: " << module << " N/um" <<
  // std::endl;

  Real3 force2on1({module * comp1, module * comp2, module * comp3});

  // real_t lhs_mass = static_cast<const Cell*>(lhs)->GetMass();

  // std::cout << "Time step lenght is " <<
  // Simulation::GetActive()->GetParam()->simulation_time_step << " min, Cell
  // mass is " << lhs_mass << " kg" << std::endl;

  // std::cout << "Displacement is " << force2on1 *
  // Simulation::GetActive()->GetParam()->simulation_time_step / lhs_mass << "
  // um, Force is " << force2on1 << " N" << std::endl;

  return {force2on1[0], force2on1[1], force2on1[2], 0};
}

}  // namespace bdm
