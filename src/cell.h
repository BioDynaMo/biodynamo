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

#ifndef CELL_H_
#define CELL_H_

#include <array>
#include <cmath>
#include <complex>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

// #include "backend.h"
// #include "biology_module_util.h"
#include "event/cell_division_event.h"
#include "event/event.h"
#include "math_util.h"
#include "param.h"
#include "shape.h"
#include "simulation_object.h"
#include "simulation.h"
#include "resource_manager.h"
// #include "simulation_object_util.h"

namespace bdm {

class Cell : public SimulationObject {
  // BDM_SIM_OBJECT_HEADER(Cell, 1, biology_modules_, position_, tractor_force_,
  //                       diameter_, volume_, adherence_, density_, box_idx_);

 public:
  /// First axis of the local coordinate system.
  static constexpr std::array<double, 3> kXAxis = {{1.0, 0.0, 0.0}};
  /// Second axis of the local coordinate system.
  static constexpr std::array<double, 3> kYAxis = {{0.0, 1.0, 0.0}};
  /// Third axis of the local coordinate system.
  static constexpr std::array<double, 3> kZAxis = {{0.0, 0.0, 1.0}};

  /// Returns the data members that are required to visualize this simulation
  /// object.
  static std::set<std::string> GetRequiredVisDataMembers() {
    return {"position_", "diameter_"};
  }

  static constexpr Shape GetShape() { return Shape::kSphere; }

  Cell() : density_(1.0) {}
  explicit Cell(double diameter) : diameter_(diameter), density_(1.0) {
    UpdateVolume();
  }
  explicit Cell(const std::array<double, 3>& position)
      : position_(position), density_{1.0} {}

  /// This constructor is used to create daughter 2 for a cell division event
  /// \see CellDivisionEvent
  Cell(const CellDivisionEvent& event, Cell* mother) {
    auto* daughter = this;  // FIXME remove daughter
    // A) Defining some values
    // ..................................................................
    // defining the two radii s.t total volume is conserved
    // * radius^3 = r1^3 + r2^3 ;
    // * volume_ratio = r2^3 / r1^3
    double radius = mother->GetDiameter() * 0.5;

    // define an axis for division (along which the nuclei will move)
    double x_coord = std::cos(event.theta_) * std::sin(event.phi_);
    double y_coord = std::sin(event.theta_) * std::sin(event.phi_);
    double z_coord = std::cos(event.phi_);
    double total_length_of_displacement = radius / 4.0;

    const auto x_axis = mother->kXAxis;
    const auto y_axis = mother->kYAxis;
    const auto z_axis = mother->kZAxis;
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
    double d_2 = total_length_of_displacement / (event.volume_ratio_ + 1);
    double d_1 = total_length_of_displacement - d_2;

    double mother_volume = mother->GetVolume();
    double new_volume = mother_volume / (event.volume_ratio_ + 1);
    daughter->SetVolume(mother_volume - new_volume);

    // position
    auto mother_pos = mother->GetPosition();
    std::array<double, 3> new_position{
        mother_pos[0] + d_2 * axis_of_division[0],
        mother_pos[1] + d_2 * axis_of_division[1],
        mother_pos[2] + d_2 * axis_of_division[2]};
    daughter->SetPosition(new_position);

    // // biology modules
    auto& mother_bms = mother->biology_modules_;
    // // copy biology_modules_ to me
    auto& my_bms = biology_modules_;
    CopyBiologyModules(event, &mother_bms, &my_bms);

    // E) This sphere becomes the 1st daughter
    // move these cells on opposite direction
    mother_pos[0] -= d_1 * axis_of_division[0];
    mother_pos[1] -= d_1 * axis_of_division[1];
    mother_pos[2] -= d_1 * axis_of_division[2];
    // update mother here and not in EventHandler to avoid recomputation
    mother->SetPosition(mother_pos);
    mother->SetVolume(new_volume);

    daughter->SetAdherence(mother->GetAdherence());
    daughter->SetDensity(mother->GetDensity());
    daughter->SetBoxIdx(mother->GetBoxIdx());
    // G) TODO(lukas) Copy the intracellular and membrane bound Substances
  }

  virtual ~Cell() {}

  Cell* New(const Event& event, SimulationObject* other, uint64_t new_oid = 0) const override {
    if (auto* sd_event = dynamic_cast<const CellDivisionEvent*>(&event)) {
      return new Cell(*sd_event, dynamic_cast<Cell*>(other));
    } else {
      Fatal("Cell", "Cell only supports CellDivisionEvent.");
      return nullptr;
    }
  }

  /// \brief EventHandler to modify the data members of this cell
  /// after a cell division.
  ///
  /// Performs the transition mother to daughter 1
  /// \param event contains parameters for cell division
  /// \param daughter_2 pointer to new cell (=daughter 2)
  /// \see CellDivisionEvent
  void EventHandler(const Event& event, SimulationObject* daughter_2) override {
    // call event handler for biology modules
    if (dynamic_cast<const CellDivisionEvent*>(&event)) {
      auto* daughter_bms = &(dynamic_cast<Cell*>(daughter_2)->biology_modules_);
      BiologyModuleEventHandler(event, &(biology_modules_), daughter_bms);
    } else {
      Fatal("Cell", "Cell only supports CellDivisionEvent.");
    }
  }

  /// \brief Divide this cell.
  ///
  /// CellDivisionEvent::volume_ratio_ will be between 0.9 and 1.1\n
  /// The axis of division is random.
  /// \see CellDivisionEvent
  Cell* Divide() {
    auto* random = Simulation::GetActive()->GetRandom();
    return Divide(random->Uniform(0.9, 1.1));
  }

  /// \brief Divide this cell.
  ///
  /// The axis of division is random.
  /// \see CellDivisionEvent
  Cell* Divide(double volume_ratio) {
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
  Cell* Divide(const std::array<double, 3>& axis) {
    auto* random = Simulation::GetActive()->GetRandom();
    auto polarcoord =
        TransformCoordinatesGlobalToPolar(Math::Add(axis, position_));
    return Divide(random->Uniform(0.9, 1.1), polarcoord[1],
                            polarcoord[2]);
  }

  /// \brief Divide this cell.
  ///
  /// \see CellDivisionEvent
  Cell* Divide(double volume_ratio,
                          const std::array<double, 3>& axis) {
    auto polarcoord =
        TransformCoordinatesGlobalToPolar(Math::Add(axis, position_));
    return Divide(volume_ratio, polarcoord[1], polarcoord[2]);
  }

  /// \brief Divide this cell.
  ///
  /// \see CellDivisionEvent
  Cell* Divide(double volume_ratio, double phi, double theta) {
    auto* rm = Simulation::GetActive()->GetResourceManager();
    CellDivisionEvent event(volume_ratio, phi, theta);
    auto* daughter = dynamic_cast<Cell*>(New(event, this));
    rm->push_back(daughter);
    EventHandler(event, daughter);
    return daughter;
  }

  double GetAdherence() const { return adherence_; }

  double GetDiameter() const override { return diameter_; }

  double GetMass() const { return density_ * volume_; }

  double GetDensity() const { return density_; }

  const std::array<double, 3>& GetPosition() const override { return position_; }

  const std::array<double, 3>& GetTractorForce() const {
    return tractor_force_;
  }

  double GetVolume() const { return volume_; }

  void SetAdherence(double adherence) { adherence_ = adherence; }

  void SetDiameter(double diameter) {
    diameter_ = diameter;
    UpdateVolume();
  }

  void SetVolume(double volume) {
    volume_ = volume;
    UpdateDiameter();
  }

  void SetMass(double mass) { density_ = mass / volume_; }

  void SetDensity(double density) { density_ = density; }

  void SetPosition(const std::array<double, 3>& position) override {
    position_ = position;
  }

  void SetTractorForce(const std::array<double, 3>& tractor_force) {
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
    diameter_ = std::cbrt(volume_ * 6 / Math::kPi);
  }

  void UpdateVolume() {
    // V = (4/3)*pi*r^3 = (pi/6)*diameter^3
    volume_ = Math::kPi / 6 * std::pow(diameter_, 3);
  }

  void UpdatePosition(const std::array<double, 3>& delta) {
    position_[0] += delta[0];
    position_[1] += delta[1];
    position_[2] += delta[2];
  }

  std::array<double, 3> CalculateDisplacement(double squared_radius) const override;

  void ApplyDisplacement(const std::array<double, 3>& displacement) override;

  // // FIXME make protected after ROOT issue has been resolved and all
  // // biology_modules_ are in one class.
  // /// collection of biology modules which define the internal behavior
  // std::vector<BiologyModules> biology_modules_;

 protected:
  /// Returns the position in the polar coordinate system (cylindrical or
  /// spherical) of a point expressed in global cartesian coordinates
  /// ([1,0,0],[0,1,0],[0,0,1]).
  /// @param coord: position in absolute coordinates - [x,y,z] cartesian values
  /// @return the position in local coordinates
  std::array<double, 3> TransformCoordinatesGlobalToPolar(
      const std::array<double, 3>& coord) const;

  std::array<double, 3> position_ = {{0, 0, 0}};
  std::array<double, 3> tractor_force_ = {{0, 0, 0}};
  double diameter_;
  double volume_;
  double adherence_;
  double density_;

  /// Grid box index
  uint32_t box_idx_;
};

}  // namespace bdm

#endif  // CELL_H_
