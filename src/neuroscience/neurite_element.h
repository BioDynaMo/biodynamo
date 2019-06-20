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

#ifndef NEUROSCIENCE_NEURITE_ELEMENT_H_
#define NEUROSCIENCE_NEURITE_ELEMENT_H_

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/default_force.h"
#include "core/shape.h"
#include "core/sim_object/sim_object.h"
#include "core/util/log.h"
#include "core/util/math.h"
#include "core/util/random.h"
#include "neuroscience/event/neurite_bifurcation_event.h"
#include "neuroscience/event/neurite_branching_event.h"
#include "neuroscience/event/new_neurite_extension_event.h"
#include "neuroscience/event/side_neurite_extension_event.h"
#include "neuroscience/event/split_neurite_element_event.h"
#include "neuroscience/neuron_or_neurite.h"
#include "neuroscience/neuron_soma.h"
#include "neuroscience/param.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

/// Class defining a neurite element with cylindrical geometry.
/// A cylinder can be seen as a normal cylinder, with two end points and a
/// diameter. It is oriented; the two points are called proximal and distal.
/// The neurite element is be part of a tree-like structure with (one and only)
/// one object at its proximal point and (up to) two neurite elements at
/// its distal end. The proximal end can be a Neurite or Neuron cell body.
/// If there is only one daughter, it is the left one.
/// If `daughter_left_ == nullptr`, there is no distal neurite element.
/// (it is a terminal neurite element). The presence of a `daughter_left_`
/// means that this branch has a bifurcation at its distal end.
/// \n
/// All the mass of the neurite element is concentrated at the distal point.
/// Only the distal end is moved. All the forces that are applied to the
/// proximal node are transmitted to the mother element
class NeuriteElement : public SimObject, public NeuronOrNeurite {
  BDM_SIM_OBJECT_HEADER(NeuriteElement, SimObject, 1, mass_location_, position_,
                        volume_, diameter_, density_, adherence_, x_axis_,
                        y_axis_, z_axis_, is_axon_, mother_, daughter_left_,
                        daughter_right_, branch_order_,
                        force_to_transmit_to_proximal_mass_, spring_axis_,
                        actual_length_, tension_, spring_constant_,
                        resting_length_);

 public:
  NeuriteElement() {
    resting_length_ =
        spring_constant_ * actual_length_ / (tension_ + spring_constant_);
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    tension_ = param->neurite_default_tension_;
    SetDiameter(param->neurite_default_diameter_);
    SetActualLength(param->neurite_default_actual_length_);
    density_ = param->neurite_default_density_;
    spring_constant_ = param->neurite_default_spring_constant_;
    adherence_ = param->neurite_default_adherence_;
    UpdateVolume();
  }

  /// TODO
  NeuriteElement(const Event& event, SimObject* other, uint64_t new_oid = 0)
      : Base(event, other, new_oid) {
    resting_length_ =
        spring_constant_ * actual_length_ / (tension_ + spring_constant_);

    if (event.GetId() == NewNeuriteExtensionEvent::kEventId) {
      const auto& e = dynamic_cast<const NewNeuriteExtensionEvent&>(event);
      auto* soma = dynamic_cast<NeuronSoma*>(other);
      InitializeNewNeuriteExtension(soma, e.diameter_, e.phi_, e.theta_);
    } else if (event.GetId() == NeuriteBifurcationEvent::kEventId) {
      const auto& e = dynamic_cast<const NeuriteBifurcationEvent&>(event);
      auto* ne = dynamic_cast<NeuriteElement*>(other);
      double diameter;
      Double3 direction;
      if (new_oid == 0) {
        // left branch
        diameter = e.diameter_left_;
        direction = e.direction_left_;
      } else {
        // right branch
        diameter = e.diameter_right_;
        direction = e.direction_right_;
      }
      InitializeNeuriteBifurcation(ne, e.length_, diameter, direction);
    } else if (event.GetId() == SideNeuriteExtensionEvent::kEventId) {
      const auto& e = dynamic_cast<const SideNeuriteExtensionEvent&>(event);
      auto* ne = dynamic_cast<NeuriteElement*>(other);
      InitializeSideExtensionOrBranching(ne, e.length_, e.diameter_,
                                         e.direction_);
    } else if (event.GetId() == SplitNeuriteElementEvent::kEventId) {
      const auto& e = dynamic_cast<const SplitNeuriteElementEvent&>(event);
      auto* ne = dynamic_cast<NeuriteElement*>(other);
      InitializeSplitOrBranching(ne, e.distal_portion_);
    } else if (event.GetId() == NeuriteBranchingEvent::kEventId) {
      const auto& e = dynamic_cast<const NeuriteBranchingEvent&>(event);
      auto* ne = dynamic_cast<NeuriteElement*>(other);
      if (new_oid == 0) {
        InitializeSplitOrBranching(ne, e.distal_portion_);
      } else {
        InitializeSideExtensionOrBranching(ne, e.length_, e.diameter_,
                                           e.direction_);
      }
    }
  }

  void EventHandler(const Event& event, SimObject* other1,
                    SimObject* other2 = nullptr) override {
    Base::EventHandler(event, other1, other2);

    if (event.GetId() == NeuriteBifurcationEvent::kEventId) {
      SetDaughterLeft(other1->GetSoPtr<NeuriteElement>());
      SetDaughterRight(other2->GetSoPtr<NeuriteElement>());
    } else if (event.GetId() == SideNeuriteExtensionEvent::kEventId) {
      SetDaughterRight(other2->GetSoPtr<NeuriteElement>());
    } else if (event.GetId() == SplitNeuriteElementEvent::kEventId) {
      const auto& e = dynamic_cast<const SplitNeuriteElementEvent&>(event);
      auto* proximal = dynamic_cast<NeuriteElement*>(other1);
      resting_length_ *= e.distal_portion_;

      // family relations
      mother_->UpdateRelative(*this, *proximal);
      mother_ = proximal->GetSoPtr<NeuronOrNeurite>();

      UpdateDependentPhysicalVariables();
      proximal->UpdateDependentPhysicalVariables();
      // UpdateLocalCoordinateAxis has to come after UpdateDepend...
      proximal->UpdateLocalCoordinateAxis();
    } else if (event.GetId() == NeuriteBranchingEvent::kEventId) {
      const auto& e = dynamic_cast<const NeuriteBranchingEvent&>(event);
      auto* proximal = dynamic_cast<NeuriteElement*>(other1);
      auto* branch = dynamic_cast<NeuriteElement*>(other2);

      // TODO(lukas) some code duplication with SplitNeuriteElementEvent and
      // SideNeuriteExtensionEvent event handler
      proximal->SetDaughterRight(branch->GetSoPtr<NeuriteElement>());

      // elongation
      resting_length_ *= e.distal_portion_;
      mother_->UpdateRelative(*this, *proximal);
      mother_ = proximal->GetSoPtr<NeuronOrNeurite>();

      UpdateDependentPhysicalVariables();
      proximal->UpdateDependentPhysicalVariables();
      // UpdateLocalCoordinateAxis has to come after UpdateDepend...
      proximal->UpdateLocalCoordinateAxis();
    }
  }

  SoUid GetUid() const override { return Base::GetUid(); }

  Shape GetShape() const override { return Shape::kCylinder; }

  /// Returns the data members that are required to visualize this simulation
  /// object.
  std::set<std::string> GetRequiredVisDataMembers() const override {
    return {"mass_location_", "diameter_", "actual_length_", "spring_axis_"};
  }

  void SetDiameter(double diameter) override {
    if (diameter > diameter_) {
      SetRunDisplacementForAllNextTs();
    }
    diameter_ = diameter;
    UpdateVolume();
  }

  void SetDensity(double density) { density_ = density; }

  const Double3& GetPosition() const override { return position_; }

  void SetPosition(const Double3& position) override {
    position_ = position;
    SetMassLocation(position + spring_axis_ * 0.5);
  }

  void UpdatePosition() { position_ = mass_location_ - (spring_axis_ * 0.5); }

  /// return end of neurite element position
  const Double3& GetMassLocation() const { return mass_location_; }

  void SetMassLocation(const Double3& mass_location) {
    mass_location_ = mass_location;
    SetRunDisplacementForAllNextTs();
  }

  double GetAdherence() const { return adherence_; }

  void SetAdherence(double adherence) { adherence_ = adherence; }

  const Double3& GetXAxis() const { return x_axis_; }
  const Double3& GetYAxis() const { return y_axis_; }
  const Double3& GetZAxis() const { return z_axis_; }

  double GetVolume() const { return volume_; }

  double GetDiameter() const override { return diameter_; }

  double GetDensity() const { return density_; }

  double GetMass() const { return density_ * volume_; }

  /// Returns the absolute coordinates of the location where the daughter is
  /// attached.
  /// @param daughter_element_idx element_idx of the daughter
  /// @return the coord
  Double3 OriginOf(SoUid daughter_uid) const override { return mass_location_; }

  // TODO(neurites) arrange in order end

  /// Retracts the neurite element, if it is a terminal one.
  /// Branch retraction by moving the distal end toward the
  /// proximal end (the mother), maintaining the same tension in the
  /// neurite element. The method shortens the actual and the resting length
  /// so that the result is a shorter neurite element with the same tension.
  ///   * If this neurite element is longer than the required shortening, it
  ///   simply retracts.
  ///   * If it is shorter and its mother has no other daughter, it merges with
  ///   it's mother and the method is recursively called (this time the cylinder
  ///   length is bigger because we have a new neurite element that resulted
  ///   from the fusion of two).
  ///   * If it is shorter and either the previous neurite element has another
  ///   daughter or the mother is not a neurite element, it disappears.
  /// @param speed the retraction speed in microns / h
  void RetractTerminalEnd(double speed) {
    // check if is a terminal branch
    if (daughter_left_ != nullptr) {
      return;
    }
    // scaling for integration step
    auto* core_param = Simulation::GetActive()->GetParam();
    speed *= core_param->simulation_time_step_;

    auto* mother_soma = dynamic_cast<NeuronSoma*>(mother_.Get());
    auto* mother_neurite = dynamic_cast<NeuriteElement*>(mother_.Get());

    if (actual_length_ > speed + 0.1) {
      // if actual_length_ > length : retraction keeping the same tension
      // (putting a limit on how short a branch can be is absolutely necessary
      //  otherwise the tension might explode)

      double new_actual_length = actual_length_ - speed;
      double factor = new_actual_length / actual_length_;
      SetActualLength(new_actual_length);
      // cf removeproximalCylinder()
      resting_length_ =
          spring_constant_ * actual_length_ / (tension_ + spring_constant_);
      SetSpringAxis(spring_axis_ * factor);

      SetMassLocation(mother_->OriginOf(Base::GetUid()) + spring_axis_);
      UpdatePosition();
      UpdateVolume();  // and update concentration of internal stuff.
    } else if (mother_soma) {
      mother_->RemoveDaughter(Base::GetSoPtr<NeuriteElement>());
      this->RemoveFromSimulation();
    } else if (mother_neurite &&
               mother_neurite->GetDaughterRight() == nullptr) {
      // if actual_length_ < length and mother is a neurite element with no
      // other daughter : merge with mother
      RemoveProximalNeuriteElement();  // also updates volume_...
      RetractTerminalEnd(speed / core_param->simulation_time_step_);
    } else {
      // if mother is neurite element with other daughter or is not a neurite
      // segment: disappear.
      mother_->RemoveDaughter(Base::GetSoPtr<NeuriteElement>());
      this->RemoveFromSimulation();

      mother_->UpdateDependentPhysicalVariables();
    }
  }

  /// Method used for active extension of a terminal branch, representing the
  /// steering of a growth cone. The movement should always be forward,
  /// otherwise no movement is performed.
  /// If `direction` points in an opposite direction than the axis, i.e.
  /// if the dot product is negative, there is no movement (only elongation is
  /// possible).
  /// @param speed
  /// @param direction
  void ElongateTerminalEnd(double speed, const Double3& direction) {
    double temp = direction * spring_axis_;
    if (temp > 0) {
      MovePointMass(speed, direction);
    }
  }

  /// Returns true if a side branch is physically possible. That is if this is
  /// not a terminal  branch and if there is not already a second daughter.
  bool BranchPermitted() const {
    return daughter_left_ != nullptr && daughter_right_ == nullptr;
  }

  /// \brief Create a branch for this neurite element.
  ///
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch(double new_branch_diameter, const Double3& direction,
                         double length = 1.0) {
    // create a new neurite element for side branch
    // we first split this neurite element into two pieces
    // then append a "daughter right" between the two
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    NeuriteBranchingEvent event(0.5, length, new_branch_diameter, direction);
    auto* proximal = GetInstance(event, this, 0);
    ctxt->push_back(proximal);
    auto* branch =
        bdm_static_cast<NeuriteElement*>(GetInstance(event, proximal, 1));
    ctxt->push_back(branch);
    EventHandler(event, proximal, branch);
    return branch;
  }

  /// \brief Create a branch for this neurite element.
  ///
  /// Diameter of new side branch will be equal to this neurites diameter.
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch(const Double3& direction) {
    return Branch(diameter_, direction);
  }

  /// \brief Create a branch for this neurite element.
  ///
  /// Use a random growth direction for the side branch.
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch(double diameter) {
    auto* random = Simulation::GetActive()->GetRandom();
    auto rand_noise = random->template UniformArray<3>(-0.1, 0.1);
    auto growth_direction = Math::Perp3(
        GetUnitaryAxisDirectionVector() + rand_noise, random->Uniform(0, 1));
    growth_direction.Normalize();
    return Branch(diameter, growth_direction);
  }

  /// \brief Create a branch for this neurite element.
  ///
  /// Use a random growth direction for the side branch.
  /// Diameter of new side branch will be equal to this neurites diameter.
  /// \see NeuriteBranchingEvent
  NeuriteElement* Branch() {
    auto* random = Simulation::GetActive()->GetRandom();
    double branch_diameter = diameter_;
    auto rand_noise = random->template UniformArray<3>(-0.1, 0.1);
    auto growth_direction = Math::Perp3(
        GetUnitaryAxisDirectionVector() + rand_noise, random->Uniform(0, 1));
    return Branch(branch_diameter, growth_direction);
  }

  /// Returns true if a bifurcation is physicaly possible. That is if the
  /// neurite element has no daughter and the actual length is bigger than the
  /// minimum required.
  bool BifurcationPermitted() const {
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    return (daughter_left_ == nullptr &&
            actual_length_ > param->neurite_minimial_bifurcation_length_);
  }

  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate(double length, double diameter_1,
                                           double diameter_2,
                                           const Double3& direction_1,
                                           const Double3& direction_2) {
    // 1) physical bifurcation
    // check it is a terminal branch
    if (daughter_left_ != nullptr) {
      Fatal("NeuriteElements",
            "Bifurcation only allowed on a terminal neurite element");
    }
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    NeuriteBifurcationEvent event(length, diameter_1, diameter_2, direction_1,
                                  direction_2);
    auto* new_branch_l =
        bdm_static_cast<NeuriteElement*>(GetInstance(event, this, 0));
    ctxt->push_back(new_branch_l);
    auto* new_branch_r =
        bdm_static_cast<NeuriteElement*>(GetInstance(event, this, 1));
    ctxt->push_back(new_branch_r);
    EventHandler(event, new_branch_l, new_branch_r);
    return {new_branch_l, new_branch_r};
  }

  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate(double diameter_1, double diameter_2,
                                           const Double3& direction_1,
                                           const Double3& direction_2);

  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate(const Double3& direction_1,
                                           const Double3& direction_2) {
    // initial default length :
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    double l = param->neurite_default_actual_length_;
    // diameters :
    double d = diameter_;
    return Bifurcate(l, d, d, direction_1, direction_2);
  }

  /// \brief Growth cone bifurcation.
  ///
  /// \see NeuriteBifurcationEvent
  std::array<NeuriteElement*, 2> Bifurcate() {
    // initial default length :
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    double l = param->neurite_default_actual_length_;
    // diameters :
    double d = diameter_;
    // direction : (60 degrees between branches)
    auto* random = Simulation::GetActive()->GetRandom();
    double random_val = random->Uniform(0, 1);
    auto perp_plane = Math::Perp3(spring_axis_, random_val);
    double angle_between_branches = Math::kPi / 3.0;
    auto direction_1 = Math::RotAroundAxis(
        spring_axis_, angle_between_branches * 0.5, perp_plane);
    auto direction_2 = Math::RotAroundAxis(
        spring_axis_, -angle_between_branches * 0.5, perp_plane);

    return Bifurcate(l, d, d, direction_1, direction_2);
  }

  // ***************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE *
  // ***************************************************************************

  // TODO(neurites) documentation
  void RemoveDaughter(const SoPointer<NeuriteElement>& daughter) override {
    // If there is another daughter than the one we want to remove,
    // we have to be sure that it will be the daughterLeft->
    if (daughter == daughter_right_) {
      daughter_right_ = nullptr;
      return;
    }

    if (daughter == daughter_left_) {
      daughter_left_ = daughter_right_;
      daughter_right_ = nullptr;
      return;
    }
    Fatal("NeuriteElement", "Given object is not a daughter!");
  }

  // TODO(neurites) add documentation
  void UpdateRelative(const NeuronOrNeurite& old_relative,
                      const NeuronOrNeurite& new_relative) override {
    if (&old_relative == &*mother_) {
      mother_ = new_relative.GetNeuronOrNeuriteSoPtr();
    } else {
      auto new_neurite_soptr =
          bdm_static_cast<const NeuriteElement*>(&new_relative)
              ->GetSoPtr<NeuriteElement>();
      if (&*daughter_left_ ==
          dynamic_cast<const NeuriteElement*>(&old_relative)) {
        daughter_left_ = new_neurite_soptr;
      } else if (&*daughter_right_ ==
                 dynamic_cast<const NeuriteElement*>(&old_relative)) {
        daughter_right_ = new_neurite_soptr;
      }
    }
  }

  /// Returns the total force that this `NeuriteElement` exerts on it's mother.
  /// It is the sum of the spring force and the part of the inter-object force
  /// computed earlier in `CalculateDisplacement`
  Double3 ForceTransmittedFromDaugtherToMother(const NeuronOrNeurite& mother) {
    if (mother_ != mother) {
      Fatal("NeuriteElement", "Given object is not the mother!");
      return {0, 0, 0};
    }

    // The inner tension is added to the external force that was computed
    // earlier.
    // (The reason for dividing by the actualLength is to normalize the
    // direction : T = T * axis/ (axis length)
    double factor = tension_ / actual_length_;
    if (factor < 0) {
      factor = 0;
    }
    return (spring_axis_ * factor) + force_to_transmit_to_proximal_mass_;
  }

  // ***************************************************************************
  //   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
  // ***************************************************************************

  /// Checks if this NeuriteElement is either too long or too short.
  ///   * too long: insert another NeuriteElement
  ///   * too short fuse it with the proximal element or even delete it
  ///
  /// Only executed for terminal neurite elements.
  void RunDiscretization() override {
    if (daughter_left_ != nullptr) {
      return;
    }

    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    auto* mother_soma = dynamic_cast<NeuronSoma*>(mother_.Get());
    auto* mother_neurite = dynamic_cast<NeuriteElement*>(mother_.Get());
    if (actual_length_ > param->neurite_max_length_) {
      if (daughter_left_ == nullptr) {  // if terminal branch :
        SplitNeuriteElement(0.1);
      } else if (mother_soma != nullptr) {  // if initial branch :
        SplitNeuriteElement(0.9);
      } else {
        SplitNeuriteElement(0.5);
      }
    } else if (actual_length_ < param->neurite_min_length_ &&
               mother_neurite != nullptr &&
               mother_neurite->GetRestingLength() <
                   param->neurite_max_length_ - resting_length_ - 1 &&
               mother_neurite->GetDaughterRight() == nullptr &&
               daughter_left_ != nullptr) {
      // if the previous branch is removed, we first remove its associated
      // NeuriteElement
      mother_neurite->RemoveFromSimulation();
      // then we remove it
      RemoveProximalNeuriteElement();
      // TODO(neurites) LB: what about ourselves??
    }
  }

  // ***************************************************************************
  //   ELONGATION, RETRACTION, BRANCHING
  // ***************************************************************************

  /// Method used for active extension of a terminal branch, representing the
  /// steering of a
  /// growth cone. There is no check for real extension (unlike in
  /// `ExtendCylinder()`` ).
  ///
  /// @param speed      of the growth rate (microns/hours).
  /// @param direction  the 3D direction of movement.
  void MovePointMass(double speed, const Double3& direction) {
    // check if is a terminal branch
    if (daughter_left_ != nullptr) {
      return;
    }

    // scaling for integration step
    auto* core_param = Simulation::GetActive()->GetParam();
    double length = speed * core_param->simulation_time_step_;
    auto dir = direction;
    auto displacement = dir.Normalize() * length;
    auto new_mass_location = displacement + mass_location_;
    // here I have to define the actual length ..........
    auto relative_ml = mother_->OriginOf(Base::GetUid());  //  change to auto&&
    SetSpringAxis(new_mass_location - relative_ml);
    SetMassLocation(new_mass_location);
    UpdatePosition();
    SetActualLength(std::sqrt(spring_axis_ * spring_axis_));
    // process of elongation : setting tension to 0 increases the resting length
    SetRestingLengthForDesiredTension(0.0);

    // some physics and computation obligations....
    UpdateVolume();  // and update concentration of internal stuff.
    UpdateLocalCoordinateAxis();
  }

  void SetRestingLengthForDesiredTension(double tension) {
    tension_ = tension;
    if (tension == 0.0) {
      resting_length_ = actual_length_;
    } else {
      // T = k*(A-R)/R --> R = k*A/(T+K)
      resting_length_ =
          spring_constant_ * actual_length_ / (tension_ + spring_constant_);
    }
  }

  /// Progressive modification of the volume. Updates the diameter.
  /// @param speed cubic micron/ h
  void ChangeVolume(double speed) {
    // scaling for integration step
    auto* core_param = Simulation::GetActive()->GetParam();
    double delta = speed * core_param->simulation_time_step_;
    volume_ += delta;

    if (volume_ <
        5.2359877E-7) {  // minimum volume_, corresponds to minimal diameter_
      volume_ = 5.2359877E-7;
    }
    UpdateDiameter();
  }

  /// Progressive modification of the diameter. Updates the volume.
  /// @param speed micron/ h
  void ChangeDiameter(double speed) {
    // scaling for integration step
    auto* core_param = Simulation::GetActive()->GetParam();
    double delta = speed * core_param->simulation_time_step_;
    diameter_ += delta;
    UpdateVolume();
  }

  // ***************************************************************************
  //   Physics
  // ***************************************************************************

  Double3 CalculateDisplacement(double squared_radius, double dt) override {
    Double3 force_on_my_point_mass{0, 0, 0};
    Double3 force_on_my_mothers_point_mass{0, 0, 0};

    // 1) Spring force
    //   Only the spring of this cylinder. The daughters spring also act on this
    //    mass, but they are treated in point (2)
    double factor = -tension_ / actual_length_;  // the minus sign is important
                                                 // because the spring axis goes
                                                 // in the opposite direction
    force_on_my_point_mass = force_on_my_point_mass + (spring_axis_ * factor);

    // 2) Force transmitted by daugthers (if they exist)
    if (daughter_left_ != nullptr) {
      auto force_from_daughter =
          daughter_left_->ForceTransmittedFromDaugtherToMother(*this);
      force_on_my_point_mass = force_on_my_point_mass + force_from_daughter;
    }
    if (daughter_right_ != nullptr) {
      auto force_from_daughter =
          daughter_right_->ForceTransmittedFromDaugtherToMother(*this);
      force_on_my_point_mass = force_on_my_point_mass + force_from_daughter;
    }

    Double3 force_from_neighbors = {0, 0, 0};

    auto* core_param = Simulation::GetActive()->GetParam();
    // this value will be used to reduce force for neurite/neurite interactions
    double h_over_m = 0.01;

    // 3) Object avoidance force
    bool has_neurite_neighbor = false;
    //  (We check for every neighbor object if they touch us, i.e. push us away)
    auto calculate_neighbor_forces = [this, &force_from_neighbors,
                                      &force_on_my_mothers_point_mass,
                                      &h_over_m, &has_neurite_neighbor](
        const SimObject* neighbor) {
      // if neighbor is a NeuriteElement
      // use shape to determine if neighbor is a NeuriteElement
      // this is much faster than using a dynamic_cast
      if (neighbor->GetShape() == Shape::kCylinder) {
        // if it is a direct relative, or sister branch, we don't take it into
        // account
        if (this->GetDaughterLeft() == *neighbor ||
            this->GetDaughterRight() == *neighbor ||
            this->GetMother() ==
                bdm_static_cast<const NeuriteElement*>(neighbor)->GetMother() ||
            (this->GetMother() == *neighbor)) {
          return;
        }
      } else if (auto* neighbor_soma =
                     dynamic_cast<const NeuronSoma*>(neighbor)) {
        // if neighbor is NeuronSoma
        // if it is a direct relative, we don't take it into account
        if (this->GetMother() == *neighbor_soma) {
          return;
        }
      }

      DefaultForce force;
      Double4 force_from_neighbor = force.GetForce(this, neighbor);

      // hack: if the neighbour is a neurite, we need to reduce the force from
      // that neighbour in order to avoid kink behaviour
      if (neighbor->GetShape() == Shape::kCylinder) {
        force_from_neighbor = force_from_neighbor * h_over_m;
        has_neurite_neighbor = true;
      }

      if (std::abs(force_from_neighbor[3]) <
          1E-10) {  // TODO(neurites) hard coded value
        // (if all the force is transmitted to the (distal end) point mass)
        force_from_neighbors[0] += force_from_neighbor[0];
        force_from_neighbors[1] += force_from_neighbor[1];
        force_from_neighbors[2] += force_from_neighbor[2];
      } else {
        // (if there is a part transmitted to the proximal end)
        double part_for_point_mass = 1.0 - force_from_neighbor[3];
        force_from_neighbors[0] += force_from_neighbor[0] * part_for_point_mass;
        force_from_neighbors[1] += force_from_neighbor[1] * part_for_point_mass;
        force_from_neighbors[2] += force_from_neighbor[2] * part_for_point_mass;
        force_on_my_mothers_point_mass[0] +=
            force_from_neighbor[0] * force_from_neighbor[3];
        force_on_my_mothers_point_mass[1] +=
            force_from_neighbor[1] * force_from_neighbor[3];
        force_on_my_mothers_point_mass[2] +=
            force_from_neighbor[2] * force_from_neighbor[3];
      }
    };

    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    ctxt->ForEachNeighborWithinRadius(calculate_neighbor_forces, *this,
                                      squared_radius);
    // hack: if the neighbour is a neurite, and as we reduced the force from
    // that neighbour, we also need to reduce my internal force (from internal
    // tension and daughters)
    if (has_neurite_neighbor) {
      force_on_my_point_mass = force_on_my_point_mass * h_over_m;
    }

    force_on_my_point_mass = force_on_my_point_mass + force_from_neighbors;

    // 5) define the force that will be transmitted to the mother
    force_to_transmit_to_proximal_mass_ = force_on_my_mothers_point_mass;
    //  6.1) Define movement scale
    double force_norm = force_on_my_point_mass.Norm();
    //  6.2) If is F not strong enough -> no movements
    if (force_norm < adherence_) {
      return {0, 0, 0};
    }

    // So, what follows is only executed if we do actually move :

    //  6.3) Since there's going be a move, we calculate it
    auto& displacement = force_on_my_point_mass;
    double& displacement_norm = force_norm;

    //  6.4) There is an upper bound for the movement.
    if (displacement_norm > core_param->simulation_max_displacement_) {
      displacement = displacement * (core_param->simulation_max_displacement_ /
                                     displacement_norm);
    }
    return displacement;
  }

  // TODO(neurites) documentation
  void ApplyDisplacement(const Double3& displacement) override {
    // FIXME comparing doubles
    if (displacement == Double3{0, 0, 0}) {
      return;
    }

    // move of our mass
    SetMassLocation(GetMassLocation() + displacement);
    // Recompute length, tension and re-center the computation node, and
    // redefine axis
    UpdateDependentPhysicalVariables();
    UpdateLocalCoordinateAxis();

    // FIXME this whole block might be superfluous - ApplyDisplacement is called
    // For the relatives: recompute the lenght, tension etc. (why for mother?
    // have to think about that)
    if (daughter_left_ != nullptr) {
      daughter_left_->UpdateDependentPhysicalVariables();
      daughter_left_->UpdateLocalCoordinateAxis();
    }
    if (daughter_right_ != nullptr) {
      daughter_right_->UpdateDependentPhysicalVariables();
      daughter_right_->UpdateLocalCoordinateAxis();
    }
  }

  /// Defines the three orthonormal local axis so that a cylindrical coordinate
  /// system can be used. The `x_axis_` is aligned with the `spring_axis_`.
  /// The two other are in the plane perpendicular to `spring_axis_`.
  /// This method to update the axis was suggested by Matt Coock.
  /// Although not perfectly exact, it is accurate enough for us to use.
  void UpdateLocalCoordinateAxis() {
    // x (new) = something new
    // z (new) = x (new) cross y(old)
    // y (new) = z(new) cross x(new)
    auto spring_axis_normalized = spring_axis_;
    x_axis_ = spring_axis_normalized.Normalize();
    z_axis_ = Math::CrossProduct(x_axis_, y_axis_);
    double norm_of_z = z_axis_.Norm();
    if (norm_of_z < 1E-10) {  // TODO(neurites) use parameter
      // If new x_axis_ and old y_axis_ are aligned, we cannot use this scheme;
      // we start by re-defining new perp vectors. Ok, we loose the previous
      // info, but this should almost never happen....
      auto* random = Simulation::GetActive()->GetRandom();
      z_axis_ = Math::Perp3(x_axis_, random->Uniform(0, 1));
    } else {
      z_axis_ = z_axis_ * (1 / norm_of_z);
    }
    y_axis_ = Math::CrossProduct(z_axis_, x_axis_);
  }

  /// Recomputes diameter after volume has changed.
  void UpdateDiameter() {
    double diameter = std::sqrt(4 / Math::kPi * volume_ / actual_length_);
    if (diameter > diameter_) {
      Base::SetRunDisplacementForAllNextTs();
    }
    diameter_ = diameter;
  }

  /// Recomputes volume, after diameter has been changed.
  void UpdateVolume() {
    volume_ = Math::kPi / 4 * diameter_ * diameter_ * actual_length_;
  }

  // ***************************************************************************
  //   Coordinates transform
  // ***************************************************************************

  /// 3 systems of coordinates :
  ///
  /// Global :   cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0)
  /// and (0,0,1)
  ///        with origin at (0,0,0).
  /// Local :    defined by orthogonal axis xAxis (=vect proximal to distal
  /// end), yAxis and zAxis,
  ///        with origin at proximal end
  /// Polar :    cylindrical coordinates [h,theta,r] with
  ///        h = first local coord (along xAxis),
  ///        theta = angle from yAxis,
  ///        r euclidian distance from xAxis;
  ///        with origin at proximal end
  ///
  ///  Note: The methods below transform POSITIONS and not DIRECTIONS !!!
  ///
  /// G -> L
  /// L -> G
  ///
  /// L -> P
  /// P -> L
  ///
  /// G -> P = G -> L, then L -> P
  /// P -> P = P -> L, then L -> G

  /// G -> L
  /// Returns the position in the local coordinate system (xAxis, yXis, zAxis)
  /// of a point expressed in global cartesian coordinates
  /// ([1,0,0],[0,1,0],[0,0,1]).
  /// @param position in global coordinates
  Double3 TransformCoordinatesGlobalToLocal(const Double3& position) const {
    auto pos = position - ProximalEnd();
    return {pos * x_axis_, pos * y_axis_, pos * z_axis_};
  }

  /// L -> G
  /// Returns the position in global cartesian coordinates
  /// ([1,0,0],[0,1,0],[0,0,1])
  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
  /// @param position in local coordinates
  Double3 TransformCoordinatesLocalToGlobal(const Double3& position) const {
    Double3 axis_0{x_axis_[0], y_axis_[0], z_axis_[0]};
    Double3 axis_1{x_axis_[1], y_axis_[1], z_axis_[1]};
    Double3 axis_2{x_axis_[2], y_axis_[2], z_axis_[2]};
    auto x = position * axis_0;
    auto y = position * axis_1;
    auto z = position * axis_2;
    Double3 glob{x, y, z};
    return glob + ProximalEnd();
  }

  ///  L -> P
  /// Returns the position in cylindrical coordinates (h,theta,r)
  /// of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
  /// @param position in local coordinates
  Double3 TransformCoordinatesLocalToPolar(const Double3& position) const {
    return {position[0], std::atan2(position[2], position[1]),
            std::sqrt(position[1] * position[1] + position[2] * position[2])};
  }

  /// P -> L
  /// Returns the position in the local coordinate system (xAxis, yXis, zAxis)
  /// of a point expressed in cylindrical coordinates (h,theta,r).
  /// @param position in local coordinates
  Double3 TransformCoordinatesPolarToLocal(const Double3& position) const {
    return {position[0], position[2] * std::cos(position[1]),
            position[2] * std::sin(position[1])};
  }

  /// P -> G :    P -> L, then L -> G
  Double3 TransformCoordinatesPolarToGlobal(
      const std::array<double, 2>& position) const {
    // the position is in cylindrical coord (h,theta,r)
    // with r being implicit (half the diameter_)
    // We thus have h (along x_axis_) and theta (the angle from the y_axis_).
    double r = 0.5 * diameter_;
    Double3 polar_position{position[0], position[1], r};
    auto local = TransformCoordinatesPolarToLocal(polar_position);
    return TransformCoordinatesLocalToGlobal(local);
  }

  /// G -> L :    G -> L, then L -> P
  Double3 TransformCoordinatesGlobalToPolar(const Double3& position) const {
    auto local = TransformCoordinatesGlobalToLocal(position);
    return TransformCoordinatesLocalToPolar(local);
  }

  // ***************************************************************************
  //   GETTERS & SETTERS
  // ***************************************************************************

  bool IsAxon() const { return is_axon_; }

  void SetAxon(bool is_axon) { is_axon_ = is_axon; }

  SoPointer<NeuronOrNeurite> GetMother() { return mother_; }

  const SoPointer<NeuronOrNeurite> GetMother() const { return mother_; }

  void SetMother(const SoPointer<NeuronOrNeurite>& mother) { mother_ = mother; }

  /// @return the (first) distal neurite element, if it exists,
  /// i.e. if this is not the terminal segment (otherwise returns nullptr).
  const SoPointer<NeuriteElement>& GetDaughterLeft() const {
    return daughter_left_;
  }

  void SetDaughterLeft(const SoPointer<NeuriteElement>& daughter) {
    daughter_left_ = daughter;
  }

  /// @return the second distal neurite element, if it exists
  /// i.e. if there is a branching point just after this element (otherwise
  /// returns nullptr).
  const SoPointer<NeuriteElement>& GetDaughterRight() const {
    return daughter_right_;
  }

  void SetDaughterRight(const SoPointer<NeuriteElement>& daughter) {
    daughter_right_ = daughter;
  }

  int GetBranchOrder() const { return branch_order_; }

  void SetBranchOrder(int branch_order) { branch_order_ = branch_order; }

  double GetActualLength() const { return actual_length_; }

  /// Should not be used, since the actual length depends on the geometry.
  void SetActualLength(double actual_length) {
    if (actual_length > actual_length_) {
      SetRunDisplacementForAllNextTs();
    }
    actual_length_ = actual_length;
  }

  double GetRestingLength() const { return resting_length_; }

  void SetRestingLength(double resting_length) {
    resting_length_ = resting_length;
  }

  const Double3& GetSpringAxis() const { return spring_axis_; }

  void SetSpringAxis(const Double3& axis) {
    SetRunDisplacementForAllNextTs();
    spring_axis_ = axis;
  }

  double GetSpringConstant() const { return spring_constant_; }

  void SetSpringConstant(double spring_constant) {
    spring_constant_ = spring_constant;
  }

  double GetTension() const { return tension_; }

  void SetTension(double tension) { tension_ = tension; }

  /// NOT A "REAL" GETTER
  /// Gets a vector of length 1, with the same direction as the SpringAxis.
  /// @return a normalized spring axis
  Double3 GetUnitaryAxisDirectionVector() const {
    double factor = 1.0 / actual_length_;
    return spring_axis_ * factor;
  }

  /// Should return yes if the PhysicalCylinder is considered a terminal branch.
  /// @return is it a terminal branch
  bool IsTerminal() const { return daughter_left_ == nullptr; }

  /// retuns the position of the proximal end, ie the position minus the spring
  /// axis.
  /// Is mainly used for paint
  Double3 ProximalEnd() const { return mass_location_ - spring_axis_; }

  /// Returns the position of the distal end == position_
  const Double3& DistalEnd() const { return mass_location_; }

  /// Returns the total (actual) length of all the neurite elements (including
  /// the one in which this method is
  /// called) before the previous branching point. Used to decide if long enough
  /// to bifurcate or branch,
  /// independently of the discretization.
  double LengthToProximalBranchingPoint() const {
    double length = actual_length_;
    if (auto* mother_neurite =
            dynamic_cast<const NeuriteElement*>(mother_.Get())) {
      if (mother_neurite->GetDaughterRight() == nullptr) {
        length += mother_neurite->LengthToProximalBranchingPoint();
      }
    }
    return length;
  }

  double GetLength() const { return actual_length_; }

  /// Returns the axis direction of a neurite element
  const Double3& GetAxis() const {
    // local coordinate x_axis_ is equal to cylinder axis
    return x_axis_;
  }

  /// Updates the spring axis, the actual length, the tension and the volume.
  ///
  /// For tension, `T = k * (aL - rL) / rL`.  k = spring constant,
  /// rL = resting length, aL = actual length. (Note the division by rL.
  /// Otherwise we could have cylinders with big aL and rL = 0).\n
  void UpdateDependentPhysicalVariables() override {
    auto relative_ml = mother_->OriginOf(Base::GetUid());
    SetSpringAxis(mass_location_ - relative_ml);
    SetActualLength(std::sqrt(spring_axis_ * spring_axis_));
    UpdatePosition();
    if (std::abs(actual_length_ - resting_length_) > 1e-13) {
      tension_ = spring_constant_ * (actual_length_ - resting_length_) /
                 resting_length_;
    } else {
      // avoid floating point rounding effects that increase the tension
      tension_ = 0.0;
    }
    UpdateVolume();
  }

  friend std::ostream& operator<<(std::ostream& str, const NeuriteElement& n) {
    auto pos = n.GetPosition();
    str << "MassLocation:     " << n.mass_location_[0] << ", "
        << n.mass_location_[1] << ", " << n.mass_location_[2] << ", "
        << std::endl;
    str << "Position:         " << pos[0] << ", " << pos[1] << ", " << pos[2]
        << ", " << std::endl;
    str << "x_axis_:          " << n.x_axis_[0] << ", " << n.x_axis_[1] << ", "
        << n.x_axis_[2] << ", " << std::endl;
    str << "y_axis_:          " << n.y_axis_[0] << ", " << n.y_axis_[1] << ", "
        << n.y_axis_[2] << ", " << std::endl;
    str << "z_axis_:          " << n.z_axis_[0] << ", " << n.z_axis_[1] << ", "
        << n.z_axis_[2] << ", " << std::endl;
    str << "spring_axis_:     " << n.spring_axis_[0] << ", "
        << n.spring_axis_[1] << ", " << n.spring_axis_[2] << ", " << std::endl;
    str << "volume_:          " << n.volume_ << std::endl;
    str << "diameter_:        " << n.diameter_ << std::endl;
    str << "is_axon_:  " << n.is_axon_ << std::endl;
    str << "branch_order_:    " << n.branch_order_ << std::endl;
    str << "actual_length_:   " << n.actual_length_ << std::endl;
    str << "tension_:  " << n.tension_ << std::endl;
    str << "spring_constant_: " << n.spring_constant_ << std::endl;
    str << "resting_length_:  " << n.resting_length_ << std::endl;
    str << "resting_length_:  " << n.resting_length_ << std::endl;
    str << "d left         :  " << n.daughter_left_ << std::endl;
    str << "d right         :  " << n.daughter_right_ << std::endl;
    auto* mother_soma = dynamic_cast<const NeuronSoma*>(n.mother_.Get());
    auto* mother_neurite = dynamic_cast<const NeuriteElement*>(n.mother_.Get());
    auto mother =
        mother_soma ? "neuron" : (mother_neurite ? "neurite" : "nullptr");
    str << "mother_           " << mother << std::endl;
    return str;
  }

 protected:
  void Copy(const NeuriteElement& rhs) {
    // TODO(neurites) adherence
    adherence_ = rhs.GetAdherence();
    //  density_
    SetDiameter(rhs.GetDiameter());  // also updates voluume
    x_axis_ = rhs.GetXAxis();
    y_axis_ = rhs.GetYAxis();
    z_axis_ = rhs.GetZAxis();

    SetSpringAxis(rhs.GetSpringAxis());
    branch_order_ = rhs.GetBranchOrder();
    spring_constant_ = rhs.GetSpringConstant();
    // TODO(neurites) what about actual length, tension and resting_length_ ??
  }

 private:
  // TODO(lukas) data members same as in cell -> resolve once ROOT-9321 has been
  // resolved
  /// mass_location_ is distal end of the cylinder
  /// NB: Use setter and don't assign values directly
  Double3 mass_location_ = {{0.0, 0.0, 0.0}};

  /// position_ is the middle point of cylinder
  Double3 position_ = {{0.0, 0.0, 0.0}};

  double volume_;
  /// NB: Use setter and don't assign values directly
  double diameter_ = 1;
  double density_;
  double adherence_;
  /// First axis of the local coordinate system equal to cylinder axis
  Double3 x_axis_ = {{1.0, 0.0, 0.0}};
  /// Second axis of the local coordinate system.
  Double3 y_axis_ = {{0.0, 1.0, 0.0}};
  /// Third axis of the local coordinate system.
  Double3 z_axis_ = {{0.0, 0.0, 1.0}};

  bool is_axon_ = false;

  /// Parent node in the neuron tree structure can be a Neurite element
  /// or cell body
  SoPointer<NeuronOrNeurite> mother_;

  /// First child node in the neuron tree structure (can only be a Neurite
  /// element)
  SoPointer<NeuriteElement> daughter_left_;
  /// Second child node in the neuron tree structure. (can only be a Neurite
  /// element)
  SoPointer<NeuriteElement> daughter_right_;

  /// number of branching points from here to the soma (root of the neuron
  /// tree-structure).
  int branch_order_ = 0;

  /// The part of the inter-object force transmitted to the mother (parent node)
  Double3 force_to_transmit_to_proximal_mass_ = {{0, 0, 0}};

  /// from the attachment point to the mass location
  /// (proximal -> distal).
  /// NB: Use setter and don't assign values directly
  Double3 spring_axis_ = {{0, 0, 0}};

  /// Real length of the PhysicalCylinder (norm of the springAxis).
  /// NB: Use setter and don't assign values directly
  double actual_length_ = 1;

  /// Tension in the cylinder spring.
  double tension_;

  /// Spring constant per distance unit (springConstant restingLength  = "real"
  /// spring constant).
  double spring_constant_;

  /// The length of the internal spring where tension would be zero.
  /// T = k*(A-R)/R --> R = k*A/(T+K)
  double resting_length_;

  /// \brief Split this neurite element into two segments.
  ///
  /// \see SplitNeuriteElementEvent
  NeuriteElement* SplitNeuriteElement(double distal_portion = 0.5) {
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    SplitNeuriteElementEvent event(distal_portion);
    auto* new_proximal_element =
        bdm_static_cast<NeuriteElement*>(GetInstance(event, this));
    ctxt->push_back(new_proximal_element);
    EventHandler(event, new_proximal_element);
    return new_proximal_element;
  }

  /// Merges two neurite elements together. The one in which the method is
  /// called phagocytes it's mother.
  void RemoveProximalNeuriteElement() {
    // The mother is removed if (a) it is a neurite element and (b) it has no
    // other daughter than
    auto* mother_neurite = dynamic_cast<NeuriteElement*>(mother_.Get());
    if (mother_neurite == nullptr ||
        mother_neurite->GetDaughterRight() != nullptr) {
      return;
    }
    // The guy we gonna remove
    auto* proximal_ne = mother_neurite;

    // Re-organisation of the PhysicalObject tree structure: by-passing
    // proximalCylinder
    proximal_ne->GetMother()->UpdateRelative(*mother_, *this);
    SetMother(mother_neurite->GetMother()->GetNeuronOrNeuriteSoPtr());

    // Keeping the same tension :
    // (we don't use updateDependentPhysicalVariables(), because we have tension
    // and want to
    // compute restingLength, and not the opposite...)
    // T = k*(A-R)/R --> R = k*A/(T+K)
    spring_axis_ = mass_location_ - mother_->OriginOf(Base::GetUid());
    SetActualLength(spring_axis_.Norm());
    resting_length_ =
        spring_constant_ * actual_length_ / (tension_ + spring_constant_);
    // .... and volume_
    UpdateVolume();
    // and local coord
    UpdateLocalCoordinateAxis();

    proximal_ne->RemoveFromSimulation();
  }

  /// \brief Extend a side neurite element and assign it to daughter right.
  ///
  /// \see SideNeuriteExtensionEvent
  NeuriteElement* ExtendSideNeuriteElement(double length, double diameter,
                                           const Double3& direction) {
    if (daughter_right_ != nullptr) {
      Fatal(
          "NeuriteElement",
          "Can't extend a side neurite since daughter_right is not a nullptr!");
    }

    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    SideNeuriteExtensionEvent event{length, diameter, direction};
    auto* new_branch =
        bdm_static_cast<NeuriteElement*>(GetInstance(event, this));
    new_branch->EventHandler(event, this);
    ctxt->push_back(new_branch);
    EventHandler(event, new_branch);
    return new_branch;
  }

  /// TODO
  void InitializeNewNeuriteExtension(NeuronSoma* soma, double diameter,
                                     double phi, double theta) {
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    tension_ = param->neurite_default_tension_;
    SetDiameter(param->neurite_default_diameter_);
    SetActualLength(param->neurite_default_actual_length_);
    density_ = param->neurite_default_density_;
    spring_constant_ = param->neurite_default_spring_constant_;
    adherence_ = param->neurite_default_adherence_;

    double radius = 0.5 * soma->GetDiameter();
    double new_length = param->neurite_default_actual_length_;
    // position in bdm.cells coord
    double x_coord = std::sin(theta) * std::cos(phi);
    double y_coord = std::sin(theta) * std::sin(phi);
    double z_coord = std::cos(theta);
    Double3 axis_direction{
        x_coord * soma->kXAxis[0] + y_coord * soma->kYAxis[0] +
            z_coord * soma->kZAxis[0],
        x_coord * soma->kXAxis[1] + y_coord * soma->kYAxis[1] +
            z_coord * soma->kZAxis[1],
        x_coord * soma->kXAxis[2] + y_coord * soma->kYAxis[2] +
            z_coord * soma->kZAxis[2]};

    // positions & axis in cartesian coord
    auto new_begin_location = soma->GetPosition() + (axis_direction * radius);
    auto new_spring_axis = axis_direction * new_length;

    auto new_mass_location = new_begin_location + new_spring_axis;

    // set attributes of new neurite segment
    SetDiameter(diameter);
    UpdateVolume();
    SetSpringAxis(new_spring_axis);

    SetMassLocation(new_mass_location);
    UpdatePosition();
    SetActualLength(new_length);
    SetRestingLengthForDesiredTension(param->neurite_default_tension_);
    UpdateLocalCoordinateAxis();

    // family relations
    SetMother(soma->GetSoPtr<NeuronOrNeurite>());
  }

  /// TODO
  void InitializeNeuriteBifurcation(NeuriteElement* mother, double length,
                                    double diameter, const Double3& direction) {
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    tension_ = param->neurite_default_tension_;
    SetDiameter(param->neurite_default_diameter_);
    SetActualLength(param->neurite_default_actual_length_);
    density_ = param->neurite_default_density_;
    spring_constant_ = param->neurite_default_spring_constant_;
    adherence_ = param->neurite_default_adherence_;

    Copy(*mother);
    SetMother(mother->GetSoPtr<NeuronOrNeurite>());

    // check that the directions are not pointing backwards
    auto dir_1 = direction;  // todo avoid cpy
    const auto& mother_spring_axis = mother->GetSpringAxis();
    if (Math::AngleRadian(mother_spring_axis, direction) > Math::kPi / 2.0) {
      auto proj = Math::ProjectionOnto(direction, mother_spring_axis);
      proj = proj * -1;
      dir_1 = direction + proj;
    }

    // mass location and spring axis
    const auto& mother_ml = mother->GetMassLocation();
    dir_1.Normalize();
    SetSpringAxis(dir_1 * length);
    SetMassLocation(mother_ml + spring_axis_);
    UpdatePosition();
    UpdateLocalCoordinateAxis();  // (important so that x_axis_ is correct)

    // physics of tension :
    SetActualLength(length);
    SetRestingLengthForDesiredTension(param->neurite_default_tension_);

    // set local coordinate axis in the new branches
    // TODO(neurites) again?? alreay done a few lines up
    UpdateLocalCoordinateAxis();

    // 2) creating the first daughter branch
    SetDiameter(diameter);
    branch_order_ = mother->GetBranchOrder() + 1;

    UpdateDependentPhysicalVariables();
  }

  /// Neurite branching is composed of neurite splitting and side neurite
  /// extension. To avoid code duplication in constructors, logic has been moved
  /// here.
  /// \see SplitNeuriteElementEvent, NeuriteBranchingEvent
  void InitializeSplitOrBranching(NeuriteElement* other,
                                  double distal_portion) {
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    tension_ = param->neurite_default_tension_;
    SetDiameter(param->neurite_default_diameter_);
    SetActualLength(param->neurite_default_actual_length_);
    density_ = param->neurite_default_density_;
    spring_constant_ = param->neurite_default_spring_constant_;
    adherence_ = param->neurite_default_adherence_;

    const auto& other_ml = other->GetMassLocation();
    const auto& other_sa = other->GetSpringAxis();
    const auto& other_rl = other->GetRestingLength();

    // TODO(neurites) reformulate to mass_location_
    auto new_position = other_ml - (other_sa * distal_portion);

    SetPosition(new_position);
    Copy(*other);
    UpdatePosition();

    // family relations
    SetMother(other->GetMother()->GetNeuronOrNeuriteSoPtr());
    SetDaughterLeft(other->GetSoPtr<NeuriteElement>());

    // physics
    resting_length_ = ((1 - distal_portion) * other_rl);
  }

  /// Neurite branching is composed of neurite splitting and side neurite
  /// extension. To avoid code duplication in constructors, logic has been moved
  /// here.
  void InitializeSideExtensionOrBranching(NeuriteElement* mother, double length,
                                          double diameter,
                                          const Double3& direction) {
    auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
    tension_ = param->neurite_default_tension_;
    SetDiameter(param->neurite_default_diameter_);
    SetActualLength(param->neurite_default_actual_length_);
    density_ = param->neurite_default_density_;
    spring_constant_ = param->neurite_default_spring_constant_;
    adherence_ = param->neurite_default_adherence_;

    Copy(*mother);

    auto dir = direction;
    auto direction_normalized = direction;
    direction_normalized.Normalize();
    const auto& mother_spring_axis = mother->GetSpringAxis();
    double angle_with_side_branch =
        Math::AngleRadian(mother_spring_axis, direction);
    if (angle_with_side_branch < 0.78 ||
        angle_with_side_branch > 2.35) {  // 45-135 degrees
      auto p = Math::CrossProduct(mother_spring_axis, direction);
      p = Math::CrossProduct(p, mother_spring_axis);
      dir = direction_normalized + p.Normalize();
    }
    // location of mass and computation center
    auto new_spring_axis = direction_normalized * length;
    const auto& mother_ml = mother->GetMassLocation();

    SetSpringAxis(new_spring_axis);
    SetMassLocation(mother_ml + new_spring_axis);
    UpdatePosition();
    // physics
    SetActualLength(length);
    SetRestingLengthForDesiredTension(param->neurite_default_tension_);
    SetDiameter(param->neurite_default_diameter_);
    UpdateLocalCoordinateAxis();
    // family relations
    SetMother(mother->GetSoPtr<NeuronOrNeurite>());

    branch_order_ = mother->GetBranchOrder() + 1;

    SetDiameter(diameter);

    // correct physical values (has to be after family relations
    UpdateDependentPhysicalVariables();
  }
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURITE_ELEMENT_H_
