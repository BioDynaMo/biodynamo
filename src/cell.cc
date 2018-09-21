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

#include "cell.h"
#include "default_force.h"
#include "simulation.h"
#include "grid.h"

namespace bdm {

constexpr std::array<double, 3> Cell::kXAxis;
constexpr std::array<double, 3> Cell::kYAxis;
constexpr std::array<double, 3> Cell::kZAxis;

// BDM_SO_DEFINE(template <typename TBiologyModule>
              // inline void CellExt)::AddBiologyModule(TBiologyModule&& module) {
  // biology_modules_.emplace_back(module);
// }
// void Cell::RunBiologyModules() {
//   RunVisitor<MostDerived> visitor(
//       static_cast<MostDerived*>(this));
//   for (auto& module : biology_modules_) {
//     visit(visitor, module);
//   }
// }

std::array<double, 3> Cell::CalculateDisplacement(double squared_radius) const {
  // Basically, the idea is to make the sum of all the forces acting
  // on the Point mass. It is stored in translationForceOnPointMass.
  // There is also a computation of the torque (only applied
  // by the daughter neurites), stored in rotationForce.

  // TODO(roman) : There might be a problem, in the sense that the biology
  // is not applied if the total Force is smaller than adherence.
  // Once, I should look at this more carefully.

  // If we detect enough forces to make us  move, we will re-schedule us
  // setOnTheSchedulerListForPhysicalObjects(false);

  // fixme why? copying
  const auto& tf = GetTractorForce();

  // the 3 types of movement that can occur
  // bool biological_translation = false;
  bool physical_translation = false;
  // bool physical_rotation = false;

  auto* param = Simulation::GetActive()->GetParam();
  double h = param->simulation_time_step_;
  std::array<double, 3> movement_at_next_step{0, 0, 0};

  // BIOLOGY :
  // 0) Start with tractor force : What the biology defined as active
  // movement------------
  movement_at_next_step[0] += h * tf[0];
  movement_at_next_step[1] += h * tf[1];
  movement_at_next_step[2] += h * tf[2];

  // PHYSICS
  // the physics force to move the point mass
  std::array<double, 3> translation_force_on_point_mass{0, 0, 0};
  // the physics force to rotate the cell
  // std::array<double, 3> rotation_force { 0, 0, 0 };

  // 1) "artificial force" to maintain the sphere in the ecm simulation
  // boundaries--------
  // 2) Spring force from my neurites (translation and
  // rotation)--------------------------
  // 3) Object avoidance force
  // -----------------------------------------------------------
  //  (We check for every neighbor object if they touch us, i.e. push us
  //  away)

  auto calculate_neighbor_forces = [&, this](auto* neighbor,
                                             auto&& neighbor_handle) {
    DefaultForce default_force;
    auto neighbor_force = default_force.GetForce(this, neighbor);
    translation_force_on_point_mass[0] += neighbor_force[0];
    translation_force_on_point_mass[1] += neighbor_force[1];
    translation_force_on_point_mass[2] += neighbor_force[2];
  };

  auto* grid = Simulation::GetActive()->GetGrid();
  grid->ForEachNeighborWithinRadius(calculate_neighbor_forces, *this,
                                    SoHandle(element_idx_), squared_radius);

  // 4) PhysicalBonds
  // How the physics influences the next displacement
  double norm_of_force = std::sqrt(
      translation_force_on_point_mass[0] * translation_force_on_point_mass[0] +
      translation_force_on_point_mass[1] * translation_force_on_point_mass[1] +
      translation_force_on_point_mass[2] * translation_force_on_point_mass[2]);

  // is there enough force to :
  //  - make us biologically move (Tractor) :
  //  - break adherence and make us translate ?
  physical_translation = norm_of_force > GetAdherence();

  assert(GetMass() != 0 && "The mass of a cell was found to be zero!");
  double mh = h / GetMass();
  // adding the physics translation (scale by weight) if important enough
  if (physical_translation) {
    // We scale the move with mass and time step
    movement_at_next_step[0] += translation_force_on_point_mass[0] * mh;
    movement_at_next_step[1] += translation_force_on_point_mass[1] * mh;
    movement_at_next_step[2] += translation_force_on_point_mass[2] * mh;

    // Performing the translation itself :

    // but we want to avoid huge jumps in the simulation, so there are
    // maximum distances possible
    auto* param = Simulation::GetActive()->GetParam();
    if (norm_of_force * mh > param->simulation_max_displacement_) {
      const auto& norm = Math::Normalize(movement_at_next_step);
      movement_at_next_step[0] = norm[0] * param->simulation_max_displacement_;
      movement_at_next_step[1] = norm[1] * param->simulation_max_displacement_;
      movement_at_next_step[2] = norm[2] * param->simulation_max_displacement_;
    }
  }
  return movement_at_next_step;
}

void Cell::ApplyDisplacement(
    const std::array<double, 3>& displacement) {
  UpdatePosition(displacement);
  // Reset biological movement to 0.
  SetTractorForce({0, 0, 0});
}

std::array<double, 3> Cell::
    TransformCoordinatesGlobalToPolar(const std::array<double, 3>& pos) const {
  auto vector_to_point = Math::Subtract(pos, position_);
  std::array<double, 3> local_cartesian{Math::Dot(kXAxis, vector_to_point),
                                        Math::Dot(kYAxis, vector_to_point),
                                        Math::Dot(kZAxis, vector_to_point)};
  double radius = std::sqrt(local_cartesian[0] * local_cartesian[0] +
                            local_cartesian[1] * local_cartesian[1] +
                            local_cartesian[2] * local_cartesian[2]);
  return {radius, std::acos(local_cartesian[2] / radius),
          std::atan2(local_cartesian[1], local_cartesian[0])};
}

}
