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

#include "core/sim_object/cell.h"

#include "core/default_force.h"
#include "core/event/cell_division_event.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/param/param.h"
#include "core/shape.h"
#include "core/util/math.h"

namespace bdm {

constexpr std::array<double, 3> Cell::kXAxis;
constexpr std::array<double, 3> Cell::kYAxis;
constexpr std::array<double, 3> Cell::kZAxis;

std::set<std::string> GetRequiredVisDataMembers() {
  return {"position_", "diameter_"};
}

Cell::Cell() : density_(1.0) {}

Cell::Cell(double diameter) : diameter_(diameter), density_(1.0) {
  UpdateVolume();
}
Cell::Cell(const std::array<double, 3>& position)
    : position_(position), density_{1.0} {}

Cell::Cell(const Event& event, SimObject* mother, uint64_t new_oid)
    : Base(event, mother, new_oid) {
  const CellDivisionEvent* cdevent =
      dynamic_cast<const CellDivisionEvent*>(&event);
  Cell* mother_cell = dynamic_cast<Cell*>(mother);
  if (cdevent && mother_cell) {
    auto* daughter = this;  // FIXME
    // A) Defining some values
    // ..................................................................
    // defining the two radii s.t total volume is conserved
    // * radius^3 = r1^3 + r2^3 ;
    // * volume_ratio = r2^3 / r1^3
    double radius = mother_cell->GetDiameter() * 0.5;

    // define an axis for division (along which the nuclei will move)
    double x_coord = std::cos(cdevent->theta_) * std::sin(cdevent->phi_);
    double y_coord = std::sin(cdevent->theta_) * std::sin(cdevent->phi_);
    double z_coord = std::cos(cdevent->phi_);
    double total_length_of_displacement = radius / 4.0;

    const auto x_axis = mother_cell->kXAxis;
    const auto y_axis = mother_cell->kYAxis;
    const auto z_axis = mother_cell->kZAxis;
    std::array<double, 3> axis_of_division{
        total_length_of_displacement *
            (x_coord * x_axis[0] + y_coord * y_axis[0] + z_coord * z_axis[0]),
        total_length_of_displacement *
            (x_coord * x_axis[1] + y_coord * y_axis[1] + z_coord * z_axis[1]),
        total_length_of_displacement *
            (x_coord * x_axis[2] + y_coord * y_axis[2] + z_coord * z_axis[2])};

    // two equations for the center displacement :
    //  1) d2/d1= v2/v1 = volume_ratio (each sphere is shifted inver.
    //  proportionally to its volume)
    //  2) d1 + d2 = TOTAL_LENGTH_OF_DISPLACEMENT
    double d_2 = total_length_of_displacement / (cdevent->volume_ratio_ + 1);
    double d_1 = total_length_of_displacement - d_2;

    double mother_volume = mother_cell->GetVolume();
    double new_volume = mother_volume / (cdevent->volume_ratio_ + 1);
    daughter->SetVolume(mother_volume - new_volume);

    // position
    auto mother_pos = mother_cell->GetPosition();
    std::array<double, 3> new_position{
        mother_pos[0] + d_2 * axis_of_division[0],
        mother_pos[1] + d_2 * axis_of_division[1],
        mother_pos[2] + d_2 * axis_of_division[2]};
    daughter->SetPosition(new_position);

    // E) This sphere becomes the 1st daughter
    // move these cells on opposite direction
    mother_pos[0] -= d_1 * axis_of_division[0];
    mother_pos[1] -= d_1 * axis_of_division[1];
    mother_pos[2] -= d_1 * axis_of_division[2];
    // update mother here and not in EventHandler to avoid recomputation
    mother_cell->SetPosition(mother_pos);
    mother_cell->SetVolume(new_volume);

    daughter->SetAdherence(mother_cell->GetAdherence());
    daughter->SetDensity(mother_cell->GetDensity());
    // G) TODO(lukas) Copy the intracellular and membrane bound Substances
  }
}

Cell::~Cell() {}

void Cell::EventHandler(const Event& event, SimObject* other1,
                        SimObject* other2) {
  Base::EventHandler(event, other1, other2);
}

Shape Cell::GetShape() const { return Shape::kSphere; }

Cell* Cell::Divide() {
  auto* random = Simulation::GetActive()->GetRandom();
  return Divide(random->Uniform(0.9, 1.1));
}

Cell* Cell::Divide(double volume_ratio) {
  // find random point on sphere (based on :
  // http://mathworld.wolfram.com/SpherePointPicking.html)
  auto* random = Simulation::GetActive()->GetRandom();
  double theta = 2 * Math::kPi * random->Uniform(0, 1);
  double phi = std::acos(2 * random->Uniform(0, 1) - 1);
  return Divide(volume_ratio, phi, theta);
}

Cell* Cell::Divide(const std::array<double, 3>& axis) {
  auto* random = Simulation::GetActive()->GetRandom();
  auto polarcoord =
      TransformCoordinatesGlobalToPolar(Math::Add(axis, position_));
  return Divide(random->Uniform(0.9, 1.1), polarcoord[1], polarcoord[2]);
}

Cell* Cell::Divide(double volume_ratio, const std::array<double, 3>& axis) {
  auto polarcoord =
      TransformCoordinatesGlobalToPolar(Math::Add(axis, position_));
  return Divide(volume_ratio, polarcoord[1], polarcoord[2]);
}

Cell* Cell::Divide(double volume_ratio, double phi, double theta) {
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  CellDivisionEvent event(volume_ratio, phi, theta);
  auto* daughter = static_cast<Cell*>(GetInstance(event, this));
  ctxt->push_back(daughter);
  EventHandler(event, daughter);
  return daughter;
}

double Cell::GetAdherence() const { return adherence_; }

double Cell::GetDiameter() const { return diameter_; }

double Cell::GetMass() const { return density_ * volume_; }

double Cell::GetDensity() const { return density_; }

const std::array<double, 3>& Cell::GetPosition() const { return position_; }

const std::array<double, 3>& Cell::GetTractorForce() const {
  return tractor_force_;
}

double Cell::GetVolume() const { return volume_; }

void Cell::SetAdherence(double adherence) { adherence_ = adherence; }

void Cell::SetDiameter(double diameter) {
  diameter_ = diameter;
  UpdateVolume();
}

void Cell::SetVolume(double volume) {
  volume_ = volume;
  UpdateDiameter();
}

void Cell::SetMass(double mass) { density_ = mass / volume_; }

void Cell::SetDensity(double density) { density_ = density; }

void Cell::SetPosition(const std::array<double, 3>& position) {
  position_ = position;
}

void Cell::SetTractorForce(const std::array<double, 3>& tractor_force) {
  tractor_force_ = tractor_force;
}

void Cell::ChangeVolume(double speed) {
  // scaling for integration step
  auto* param = Simulation::GetActive()->GetParam();
  double delta = speed * param->simulation_time_step_;
  volume_ += delta;
  if (volume_ < 5.2359877E-7) {
    volume_ = 5.2359877E-7;
  }
  UpdateDiameter();
}

void Cell::UpdateDiameter() {
  // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
  diameter_ = std::cbrt(volume_ * 6 / Math::kPi);
}

void Cell::UpdateVolume() {
  // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
  volume_ = Math::kPi / 6 * std::pow(diameter_, 3);
}

void Cell::UpdatePosition(const std::array<double, 3>& delta) {
  position_[0] += delta[0];
  position_[1] += delta[1];
  position_[2] += delta[2];
}

std::array<double, 3> Cell::CalculateDisplacement(double squared_radius) {
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

  auto calculate_neighbor_forces = [&, this](const auto* neighbor) {
    DefaultForce default_force;
    auto neighbor_force = default_force.GetForce(this, neighbor);
    translation_force_on_point_mass[0] += neighbor_force[0];
    translation_force_on_point_mass[1] += neighbor_force[1];
    translation_force_on_point_mass[2] += neighbor_force[2];
  };

  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  ctxt->ForEachNeighborWithinRadius(calculate_neighbor_forces, *this,
                                    squared_radius);

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

void Cell::ApplyDisplacement(const std::array<double, 3>& displacement) {
  UpdatePosition(displacement);
  // Reset biological movement to 0.
  SetTractorForce({0, 0, 0});
}

std::array<double, 3> Cell::TransformCoordinatesGlobalToPolar(
    const std::array<double, 3>& pos) const {
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

}  // namespace bdm
