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
#include <set>
#include <string>
#include <vector>

#include "core/sim_object/sim_object.h"

namespace bdm {

class Cell : public SimObject {
  BDM_SIM_OBJECT_HEADER(Cell, SimObject, 1, position_, tractor_force_,
                        diameter_, volume_, adherence_, density_);

 public:
  /// First axis of the local coordinate system.
  static constexpr std::array<double, 3> kXAxis = {{1.0, 0.0, 0.0}};
  /// Second axis of the local coordinate system.
  static constexpr std::array<double, 3> kYAxis = {{0.0, 1.0, 0.0}};
  /// Third axis of the local coordinate system.
  static constexpr std::array<double, 3> kZAxis = {{0.0, 0.0, 1.0}};

  /// Returns the data members that are required to visualize this simulation
  /// object.
  static std::set<std::string> GetRequiredVisDataMembers();

  Cell();
  explicit Cell(double diameter);
  explicit Cell(const std::array<double, 3>& position);

  /// \brief This constructor is used to initialise the values of daughter
  /// 2 for a cell division event.
  ///
  /// \see CellDivisionEvent
  Cell(const Event& event, SimObject* mother, uint64_t new_oid = 0);

  virtual ~Cell();

  /// \brief EventHandler to modify the data members of this cell
  /// after a cell division.
  ///
  /// Performs the transition mother to daughter 1
  /// \param event contains parameters for cell division
  /// \param daughter_2 pointer to new cell (=daughter 2)
  /// \see Event, CellDivisionEvent
  void EventHandler(const Event& event, SimObject *other1, SimObject* other2 = nullptr) override;

  Shape GetShape() const override;

  /// \brief Divide this cell.
  ///
  /// CellDivisionEvent::volume_ratio_ will be between 0.9 and 1.1\n
  /// The axis of division is random.
  /// \see CellDivisionEvent
  virtual Cell* Divide();

  /// \brief Divide this cell.
  ///
  /// The axis of division is random.
  /// \see CellDivisionEvent
  virtual Cell* Divide(double volume_ratio);

  /// \brief Divide this cell.
  ///
  /// CellDivisionEvent::volume_ratio_ will be between 0.9 and 1.1\n
  /// \see CellDivisionEvent
  virtual Cell* Divide(const std::array<double, 3>& axis);

  /// \brief Divide this cell.
  ///
  /// \see CellDivisionEvent
  virtual Cell* Divide(double volume_ratio,
                          const std::array<double, 3>& axis);

  /// \brief Divide this cell.
  ///
  /// \see CellDivisionEvent
  virtual Cell* Divide(double volume_ratio, double phi, double theta);

  double GetAdherence() const;

  double GetDiameter() const override;

  double GetMass() const;

  double GetDensity() const;

  const std::array<double, 3>& GetPosition() const override;

  const std::array<double, 3>& GetTractorForce() const;

  double GetVolume() const;

  void SetAdherence(double adherence);

  void SetDiameter(double diameter) override;

  void SetVolume(double volume);

  void SetMass(double mass);

  void SetDensity(double density);

  void SetPosition(const std::array<double, 3>& position) override;

  void SetTractorForce(const std::array<double, 3>& tractor_force);

  void ChangeVolume(double speed);

  void UpdateDiameter();

  void UpdateVolume();

  void UpdatePosition(const std::array<double, 3>& delta);

  std::array<double, 3> CalculateDisplacement(double squared_radius) override;

  void ApplyDisplacement(const std::array<double, 3>& displacement) override;

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
  double diameter_ = 0;
  double volume_ = 0;
  double adherence_ = 0;
  double density_ = 0;
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_CELL_H_
