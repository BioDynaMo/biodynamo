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

#ifndef CORE_SIM_OBJECT_CELL_H_
#define CORE_SIM_OBJECT_CELL_H_

#include <array>
#include <cmath>
#include <complex>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "core/container/inline_vector.h"
#include "core/container/math_array.h"
#include "core/default_force.h"
#include "core/event/cell_division_event.h"
#include "core/event/event.h"
#include "core/execution_context/in_place_exec_ctxt.h"
#include "core/param/param.h"
#include "core/shape.h"
#include "core/sim_object/sim_object.h"
#include "core/util/math.h"

namespace bdm {

class Cell : public SimObject {
  BDM_SIM_OBJECT_HEADER(Cell, SimObject, 1, position_, tractor_force_,
                        diameter_, volume_, adherence_, density_);

 public:
  /// First axis of the local coordinate system.
  static const Double3 kXAxis;
  /// Second axis of the local coordinate system.
  static const Double3 kYAxis;
  /// Third axis of the local coordinate system.
  static const Double3 kZAxis;

  Cell() : density_(1.0) {}
  explicit Cell(double diameter) : diameter_(diameter), density_(1.0) {
    UpdateVolume();
  }
  explicit Cell(const Double3& position) : position_(position), density_{1.0} {}

  /// \brief This constructor is used to initialise the values of daughter
  /// 2 for a cell division event.
  ///
  /// \see CellDivisionEvent
  Cell(const Event& event, SimObject* mother, uint64_t new_oid = 0)
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
      Double3 coords = {x_coord, y_coord, z_coord};
      double total_length_of_displacement = radius / 4.0;

      const auto x_axis = mother_cell->kXAxis;
      const auto y_axis = mother_cell->kYAxis;
      const auto z_axis = mother_cell->kZAxis;

      Double3 axis_of_division =
          (coords.EntryWiseProduct(x_axis) + coords.EntryWiseProduct(y_axis) +
           coords.EntryWiseProduct(z_axis)) *
          total_length_of_displacement;

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
      auto new_position = mother_pos + (axis_of_division * d_2);
      daughter->SetPosition(new_position);

      // E) This sphere becomes the 1st daughter
      // move these cells on opposite direction
      mother_pos -= axis_of_division * d_1;
      // update mother here and not in EventHandler to avoid recomputation
      mother_cell->SetPosition(mother_pos);
      mother_cell->SetVolume(new_volume);

      daughter->SetAdherence(mother_cell->GetAdherence());
      daughter->SetDensity(mother_cell->GetDensity());
      // G) TODO(lukas) Copy the intracellular and membrane bound Substances
    }
  }

  virtual ~Cell() {}

  /// \brief EventHandler to modify the data members of this cell
  /// after a cell division.
  ///
  /// Performs the transition mother to daughter 1
  /// \param event contains parameters for cell division
  /// \param daughter_2 pointer to new cell (=daughter 2)
  /// \see Event, CellDivisionEvent
  void EventHandler(const Event& event, SimObject* other1,
                    SimObject* other2 = nullptr) override {
    Base::EventHandler(event, other1, other2);
  }

  Shape GetShape() const override { return Shape::kSphere; }

  /// \brief Divide this cell.
  ///
  /// CellDivisionEvent::volume_ratio_ will be between 0.9 and 1.1\n
  /// The axis of division is random.
  /// \see CellDivisionEvent
  virtual Cell* Divide() {
    auto* random = Simulation::GetActive()->GetRandom();
    return Divide(random->Uniform(0.9, 1.1));
  }

  /// \brief Divide this cell.
  ///
  /// The axis of division is random.
  /// \see CellDivisionEvent
  virtual Cell* Divide(double volume_ratio) {
    // find random point on sphere (based on :
    // http://mathworld.wolfram.com/SpherePointPicking.html)
    auto* random = Simulation::GetActive()->GetRandom();
    double theta = 2 * Math::kPi * random->Uniform(0, 1);
    double phi = std::acos(2 * random->Uniform(0, 1) - 1);
    return Divide(volume_ratio, phi, theta);
  }

  /// \brief Divide this cell.
  ///
  /// CellDivisionEvent::volume_ratio_ will be between 0.9 and 1.1\n
  /// \see CellDivisionEvent
  virtual Cell* Divide(const Double3& axis) {
    auto* random = Simulation::GetActive()->GetRandom();
    auto polarcoord = TransformCoordinatesGlobalToPolar(axis + position_);
    return Divide(random->Uniform(0.9, 1.1), polarcoord[1], polarcoord[2]);
  }

  /// \brief Divide this cell.
  ///
  /// \see CellDivisionEvent
  virtual Cell* Divide(double volume_ratio, const Double3& axis) {
    auto polarcoord = TransformCoordinatesGlobalToPolar(axis + position_);
    return Divide(volume_ratio, polarcoord[1], polarcoord[2]);
  }

  /// \brief Divide this cell.
  ///
  /// \see CellDivisionEvent
  virtual Cell* Divide(double volume_ratio, double phi, double theta) {
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    CellDivisionEvent event(volume_ratio, phi, theta);
    auto* daughter = static_cast<Cell*>(GetInstance(event, this));
    ctxt->push_back(daughter);
    EventHandler(event, daughter);
    return daughter;
  }

  double GetAdherence() const { return adherence_; }

  double GetDiameter() const override { return diameter_; }

  double GetMass() const { return density_ * volume_; }

  double GetDensity() const { return density_; }

  const Double3& GetPosition() const override { return position_; }

  const Double3& GetTractorForce() const { return tractor_force_; }

  double GetVolume() const { return volume_; }

  void SetAdherence(double adherence) { adherence_ = adherence; }

  void SetDiameter(double diameter) override {
    if (diameter > diameter_) {
      SetRunDisplacementForAllNextTs();
    }
    diameter_ = diameter;
    UpdateVolume();
  }

  void SetVolume(double volume) {
    volume_ = volume;
    UpdateDiameter();
  }

  void SetMass(double mass) { density_ = mass / volume_; }

  void SetDensity(double density) { density_ = density; }

  void SetPosition(const Double3& position) override {
    position_ = position;
    SetRunDisplacementForAllNextTs();
  }

  void SetTractorForce(const Double3& tractor_force) {
    tractor_force_ = tractor_force;
  }

  void ChangeVolume(double speed) {
    // scaling for integration step
    auto* param = Simulation::GetActive()->GetParam();
    double delta = speed * param->simulation_time_step_;
    volume_ += delta;
    if (volume_ < 5.2359877E-7) {
      volume_ = 5.2359877E-7;
    }
    UpdateDiameter();
  }

  void UpdateDiameter() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    double diameter = std::cbrt(volume_ * 6 / Math::kPi);
    if (diameter > diameter_) {
      Base::SetRunDisplacementForAllNextTs();
    }
    diameter_ = diameter;
  }

  void UpdateVolume() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    volume_ = Math::kPi / 6 * std::pow(diameter_, 3);
  }

  void UpdatePosition(const Double3& delta) {
    position_ += delta;
    SetRunDisplacementForAllNextTs();
  }

  Double3 CalculateDisplacement(double squared_radius, double dt) override {
    // Basically, the idea is to make the sum of all the forces acting
    // on the Point mass. It is stored in translationForceOnPointMass.
    // There is also a computation of the torque (only applied
    // by the daughter neurites), stored in rotationForce.

    // TODO(roman) : There might be a problem, in the sense that the biology
    // is not applied if the total Force is smaller than adherence.
    // Once, I should look at this more carefully.

    // fixme why? copying
    const auto& tf = GetTractorForce();

    // the 3 types of movement that can occur
    // bool biological_translation = false;
    bool physical_translation = false;
    // bool physical_rotation = false;

    double h = dt;
    Double3 movement_at_next_step{0, 0, 0};

    // BIOLOGY :
    // 0) Start with tractor force : What the biology defined as active
    // movement------------
    movement_at_next_step += tf * h;

    // PHYSICS
    // the physics force to move the point mass
    Double3 translation_force_on_point_mass{0, 0, 0};
    // the physics force to rotate the cell
    // Double3 rotation_force { 0, 0, 0 };

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
    double norm_of_force = std::sqrt(translation_force_on_point_mass *
                                     translation_force_on_point_mass);

    // is there enough force to :
    //  - make us biologically move (Tractor) :
    //  - break adherence and make us translate ?
    physical_translation = norm_of_force > GetAdherence();

    assert(GetMass() != 0 && "The mass of a cell was found to be zero!");
    double mh = h / GetMass();
    // adding the physics translation (scale by weight) if important enough
    if (physical_translation) {
      // We scale the move with mass and time step
      movement_at_next_step += translation_force_on_point_mass * mh;

      // Performing the translation itself :
      // but we want to avoid huge jumps in the simulation, so there are
      // maximum distances possible
      auto* param = Simulation::GetActive()->GetParam();
      if (norm_of_force * mh > param->simulation_max_displacement_) {
        movement_at_next_step.Normalize();
        movement_at_next_step *= param->simulation_max_displacement_;
      }
    }
    return movement_at_next_step;
  }

  void ApplyDisplacement(const Double3& displacement) override;

 protected:
  /// Returns the position in the polar coordinate system (cylindrical or
  /// spherical) of a point expressed in global cartesian coordinates
  /// ([1,0,0],[0,1,0],[0,0,1]).
  /// @param coord: position in absolute coordinates - [x,y,z] cartesian values
  /// @return the position in local coordinates
  Double3 TransformCoordinatesGlobalToPolar(const Double3& coord) const;

  /// NB: Use setter and don't assign values directly
  Double3 position_ = {{0, 0, 0}};
  Double3 tractor_force_ = {{0, 0, 0}};
  /// NB: Use setter and don't assign values directly
  double diameter_ = 0;
  double volume_ = 0;
  double adherence_ = 0;
  double density_ = 0;
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_CELL_H_
