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

#include "neuroscience/neurite_element.h"

#include "core/default_force.h"
#include "core/param/param.h"
#include "core/shape.h"
#include "core/util/log.h"
#include "core/util/math.h"
#include "core/util/random.h"
#include "neuroscience/event/neurite_bifurcation_event.h"
#include "neuroscience/event/neurite_branching_event.h"
#include "neuroscience/event/new_neurite_extension_event.h"
#include "neuroscience/event/side_neurite_extension_event.h"
#include "neuroscience/event/split_neurite_element_event.h"
#include "neuroscience/neuron_soma.h"
#include "neuroscience/param.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

std::set<std::string> NeuriteElement::GetRequiredVisDataMembers() {
  return {"mass_location_", "diameter_", "actual_length_", "spring_axis_"};
}

NeuriteElement::NeuriteElement() {
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  tension_ = param->neurite_default_tension_;
  diameter_ = param->neurite_default_diameter_;
  actual_length_ = param->neurite_default_actual_length_;
  density_ = param->neurite_default_density_;
  spring_constant_ = param->neurite_default_spring_constant_;
  adherence_ = param->neurite_default_adherence_;
  UpdateVolume();
}

NeuriteElement::NeuriteElement(const Event& event, SimObject* other,
                               uint64_t new_oid)
    : Base(event, other, new_oid) {
  if (event.GetId() == NewNeuriteExtensionEvent::kEventId) {
    const auto& e = dynamic_cast<const NewNeuriteExtensionEvent&>(event);
    auto* soma = dynamic_cast<NeuronSoma*>(other);
    InitializeNewNeuriteExtension(soma, e.diameter_, e.phi_, e.theta_);
  } else if (event.GetId() == NeuriteBifurcationEvent::kEventId) {
    const auto& e = dynamic_cast<const NeuriteBifurcationEvent&>(event);
    auto* ne = dynamic_cast<NeuriteElement*>(other);
    double diameter;
    std::array<double, 3> direction;
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

void NeuriteElement::EventHandler(const Event& event, SimObject* other1,
                                  SimObject* other2) {
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

Shape NeuriteElement::GetShape() const { return Shape::kCylinder; }

void NeuriteElement::SetDiameter(double diameter) {
  diameter_ = diameter;
  UpdateVolume();
}

void NeuriteElement::SetDensity(double density) { density_ = density; }

const std::array<double, 3>& NeuriteElement::GetPosition() const {
  tmp_position_ =
      Math::Subtract(mass_location_, Math::ScalarMult(0.5, spring_axis_));
  return tmp_position_;
}

void NeuriteElement::SetPosition(const std::array<double, 3>& position) {
  mass_location_ = Math::Add(position, Math::ScalarMult(0.5, spring_axis_));
}

const std::array<double, 3>& NeuriteElement::GetMassLocation() const {
  return mass_location_;
}

void NeuriteElement::SetMassLocation(
    const std::array<double, 3>& mass_location) {
  mass_location_ = mass_location;
}

double NeuriteElement::GetAdherence() const { return adherence_; }

void NeuriteElement::SetAdherence(double adherence) { adherence_ = adherence; }

const std::array<double, 3>& NeuriteElement::GetXAxis() const {
  return x_axis_;
}
const std::array<double, 3>& NeuriteElement::GetYAxis() const {
  return y_axis_;
}
const std::array<double, 3>& NeuriteElement::GetZAxis() const {
  return z_axis_;
}

double NeuriteElement::GetVolume() const { return volume_; }

double NeuriteElement::GetDiameter() const { return diameter_; }

double NeuriteElement::GetDensity() const { return density_; }

double NeuriteElement::GetMass() const { return density_ * volume_; }

std::array<double, 3> NeuriteElement::OriginOf(SoUid daughter_uid) const {
  return mass_location_;
}

void NeuriteElement::RetractTerminalEnd(double speed) {
  // check if is a terminal branch
  if (daughter_left_ != nullptr) {
    return;
  }
  // scaling for integration step
  auto* core_param = Simulation::GetActive()->GetParam();
  speed *= core_param->simulation_time_step_;

  auto* mother_soma = mother_->As<NeuronSoma>();
  auto* mother_neurite = mother_->As<NeuriteElement>();

  if (actual_length_ > speed + 0.1) {
    // if actual_length_ > length : retraction keeping the same tension
    // (putting a limit on how short a branch can be is absolutely necessary
    //  otherwise the tension might explode)

    double new_actual_length = actual_length_ - speed;
    double factor = new_actual_length / actual_length_;
    actual_length_ = new_actual_length;
    // cf removeproximalCylinder()
    resting_length_ =
        spring_constant_ * actual_length_ / (tension_ + spring_constant_);
    spring_axis_ = Math::ScalarMult(factor, spring_axis_);

    mass_location_ = Math::Add(mother_->OriginOf(Base::GetUid()), spring_axis_);
    UpdateVolume();  // and update concentration of internal stuff.
  } else if (mother_soma) {
    mother_->RemoveDaughter(Base::GetSoPtr<NeuriteElement>());
    this->RemoveFromSimulation();
  } else if (mother_neurite && mother_neurite->GetDaughterRight() == nullptr) {
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

void NeuriteElement::ElongateTerminalEnd(
    double speed, const std::array<double, 3>& direction) {
  double temp = Math::Dot(direction, spring_axis_);
  if (temp > 0) {
    MovePointMass(speed, direction);
  }
}

bool NeuriteElement::BranchPermitted() const {
  return daughter_left_ != nullptr && daughter_right_ == nullptr;
}

NeuriteElement* NeuriteElement::Branch(double new_branch_diameter,
                                       const std::array<double, 3>& direction,
                                       double length) {
  // create a new neurite element for side branch
  // we first split this neurite element into two pieces
  // then append a "daughter right" between the two
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  NeuriteBranchingEvent event(0.5, length, new_branch_diameter, direction);
  auto* proximal = GetInstance(event, this, 0);
  ctxt->push_back(proximal);
  auto* branch = GetInstance(event, proximal, 1)->As<NeuriteElement>();
  ctxt->push_back(branch);
  EventHandler(event, proximal, branch);
  return branch;
}

NeuriteElement* NeuriteElement::Branch(const std::array<double, 3>& direction) {
  return Branch(diameter_, direction);
}

NeuriteElement* NeuriteElement::Branch(double diameter) {
  auto* random = Simulation::GetActive()->GetRandom();
  auto rand_noise = random->template UniformArray<3>(-0.1, 0.1);
  auto growth_direction =
      Math::Perp3(Math::Add(GetUnitaryAxisDirectionVector(), rand_noise),
                  random->Uniform(0, 1));
  growth_direction = Math::Normalize(growth_direction);
  return Branch(diameter, growth_direction);
}

NeuriteElement* NeuriteElement::Branch() {
  auto* random = Simulation::GetActive()->GetRandom();
  double branch_diameter = diameter_;
  auto rand_noise = random->template UniformArray<3>(-0.1, 0.1);
  auto growth_direction =
      Math::Perp3(Math::Add(GetUnitaryAxisDirectionVector(), rand_noise),
                  random->Uniform(0, 1));
  return Branch(branch_diameter, growth_direction);
}

bool NeuriteElement::BifurcationPermitted() const {
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  return (daughter_left_ == nullptr &&
          actual_length_ > param->neurite_minimial_bifurcation_length_);
}

std::array<NeuriteElement*, 2> NeuriteElement::Bifurcate(
    double length, double diameter_1, double diameter_2,
    const std::array<double, 3>& direction_1,
    const std::array<double, 3>& direction_2) {
  // 1) physical bifurcation
  // check it is a terminal branch
  if (daughter_left_ != nullptr) {
    Fatal("NeuriteElements",
          "Bifurcation only allowed on a terminal neurite element");
  }
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  NeuriteBifurcationEvent event(length, diameter_1, diameter_2, direction_1,
                                direction_2);
  auto* new_branch_l = GetInstance(event, this, 0)->As<NeuriteElement>();
  ctxt->push_back(new_branch_l);
  auto* new_branch_r = GetInstance(event, this, 1)->As<NeuriteElement>();
  ctxt->push_back(new_branch_r);
  EventHandler(event, new_branch_l, new_branch_r);
  return {new_branch_l, new_branch_r};
}

std::array<NeuriteElement*, 2> NeuriteElement::Bifurcate(
    const std::array<double, 3>& direction_1,
    const std::array<double, 3>& direction_2) {
  // initial default length :
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  double l = param->neurite_default_actual_length_;
  // diameters :
  double d = diameter_;
  return Bifurcate(l, d, d, direction_1, direction_2);
}

std::array<NeuriteElement*, 2> NeuriteElement::Bifurcate() {
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

void NeuriteElement::RemoveDaughter(const SoPointer<NeuriteElement>& daughter) {
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

void NeuriteElement::UpdateRelative(const NeuronOrNeurite& old_relative,
                                    const NeuronOrNeurite& new_relative) {
  if (&old_relative == &*mother_) {
    mother_ = new_relative.GetNeuronOrNeuriteSoPtr();
  } else {
    auto new_neurite_soptr =
        new_relative.As<NeuriteElement>()->GetSoPtr<NeuriteElement>();
    if (&*daughter_left_ == old_relative.As<NeuriteElement>()) {
      daughter_left_ = new_neurite_soptr;
    } else if (&*daughter_right_ == old_relative.As<NeuriteElement>()) {
      daughter_right_ = new_neurite_soptr;
    }
  }
}

std::array<double, 3> NeuriteElement::ForceTransmittedFromDaugtherToMother(
    const NeuronOrNeurite& mother) {
  if (mother_ != mother) {
    Fatal("NeuriteElement", "Given object is not the mother!");
  }

  // The inner tension is added to the external force that was computed
  // earlier.
  // (The reason for dividing by the actualLength is to normalize the
  // direction : T = T * axis/ (axis length)
  double factor = tension_ / actual_length_;
  if (factor < 0) {
    factor = 0;
  }
  return Math::Add(Math::ScalarMult(factor, spring_axis_),
                   force_to_transmit_to_proximal_mass_);
}

// ***************************************************************************
//   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
// ***************************************************************************

void NeuriteElement::RunDiscretization() {
  if (daughter_left_ != nullptr) {
    return;
  }

  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  auto* mother_soma = mother_->As<NeuronSoma>();
  auto* mother_neurite = mother_->As<NeuriteElement>();
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

void NeuriteElement::MovePointMass(double speed,
                                   const std::array<double, 3>& direction) {
  // check if is a terminal branch
  if (daughter_left_ != nullptr) {
    return;
  }

  // scaling for integration step
  auto* core_param = Simulation::GetActive()->GetParam();
  double length = speed * core_param->simulation_time_step_;
  auto displacement = Math::ScalarMult(length, Math::Normalize(direction));
  auto new_mass_location = Math::Add(displacement, mass_location_);
  // here I have to define the actual length ..........
  auto relative_ml = mother_->OriginOf(Base::GetUid());  //  change to auto&&
  spring_axis_ = Math::Subtract(new_mass_location, relative_ml);
  mass_location_ = new_mass_location;
  actual_length_ = std::sqrt(Math::Dot(spring_axis_, spring_axis_));
  // process of elongation : setting tension to 0 increases the resting length
  SetRestingLengthForDesiredTension(0.0);

  // some physics and computation obligations....
  UpdateVolume();  // and update concentration of internal stuff.
  UpdateLocalCoordinateAxis();
}

void NeuriteElement::SetRestingLengthForDesiredTension(double tension) {
  tension_ = tension;
  if (tension == 0.0) {
    resting_length_ = actual_length_;
  } else {
    // T = k*(A-R)/R --> R = k*A/(T+K)
    resting_length_ =
        spring_constant_ * actual_length_ / (tension_ + spring_constant_);
  }
}

void NeuriteElement::ChangeVolume(double speed) {
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

void NeuriteElement::ChangeDiameter(double speed) {
  // scaling for integration step
  auto* core_param = Simulation::GetActive()->GetParam();
  double delta = speed * core_param->simulation_time_step_;
  diameter_ += delta;
  UpdateVolume();
}

// ***************************************************************************
//   Physics
// ***************************************************************************

std::array<double, 3> NeuriteElement::CalculateDisplacement(
    double squared_radius) {
  std::array<double, 3> force_on_my_point_mass{0, 0, 0};
  std::array<double, 3> force_on_my_mothers_point_mass{0, 0, 0};

  // 1) Spring force
  //   Only the spring of this cylinder. The daughters spring also act on this
  //    mass, but they are treated in point (2)
  double factor = -tension_ / actual_length_;  // the minus sign is important
                                               // because the spring axis goes
                                               // in the opposite direction
  force_on_my_point_mass =
      Math::Add(force_on_my_point_mass, Math::ScalarMult(factor, spring_axis_));

  // 2) Force transmitted by daugthers (if they exist)
  if (daughter_left_ != nullptr) {
    auto force_from_daughter =
        daughter_left_->ForceTransmittedFromDaugtherToMother(*this);
    force_on_my_point_mass =
        Math::Add(force_on_my_point_mass, force_from_daughter);
  }
  if (daughter_right_ != nullptr) {
    auto force_from_daughter =
        daughter_right_->ForceTransmittedFromDaugtherToMother(*this);
    force_on_my_point_mass =
        Math::Add(force_on_my_point_mass, force_from_daughter);
  }

  std::array<double, 3> force_from_neighbors = {0, 0, 0};

  auto* core_param = Simulation::GetActive()->GetParam();
  // this value will be used to reduce force for neurite/neurite interactions
  double h_over_m = 0.01;

  // 3) Object avoidance force
  bool has_neurite_neighbor = false;
  //  (We check for every neighbor object if they touch us, i.e. push us away)
  auto calculate_neighbor_forces = [this, &force_from_neighbors,
                                    &force_on_my_mothers_point_mass, &h_over_m,
                                    &has_neurite_neighbor](
      const SimObject* neighbor) {
    // if neighbor is a NeuriteElement
    if (auto* neighbor_neurite = neighbor->As<NeuriteElement>()) {
      // if it is a direct relative, or sister branch, we don't take it into
      // account
      if (this->GetDaughterLeft() == *neighbor_neurite ||
          this->GetDaughterRight() == *neighbor_neurite ||
          this->GetMother() == neighbor_neurite->GetMother() ||
          (this->GetMother()->As<NeuriteElement>() &&
           this->GetMother()->As<NeuriteElement>() == neighbor_neurite)) {
        return;
      }
    } else if (auto* neighbor_soma = neighbor->As<NeuronSoma>()) {
      // if neighbor is NeuronSoma
      // if it is a direct relative, we don't take it into account
      if (this->GetMother()->As<NeuronSoma>() &&
          this->GetMother()->As<NeuronSoma>() == neighbor_soma) {
        return;
      }
    }

    DefaultForce force;
    std::array<double, 4> force_from_neighbor = force.GetForce(this, neighbor);

    // hack: if the neighbour is a neurite, we need to reduce the force from
    // that neighbour in order to avoid kink behaviour
    if (neighbor->As<NeuriteElement>() != nullptr) {
      force_from_neighbor = Math::ScalarMult(h_over_m, force_from_neighbor);
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
    force_on_my_point_mass = Math::ScalarMult(h_over_m, force_on_my_point_mass);
  }

  force_on_my_point_mass =
      Math::Add(force_on_my_point_mass, force_from_neighbors);

  // 5) define the force that will be transmitted to the mother
  force_to_transmit_to_proximal_mass_ = force_on_my_mothers_point_mass;
  //  6.1) Define movement scale
  double force_norm = Math::Norm(force_on_my_point_mass);
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
    displacement = Math::ScalarMult(
        core_param->simulation_max_displacement_ / displacement_norm,
        displacement);
  }

  return displacement;
}

void NeuriteElement::ApplyDisplacement(
    const std::array<double, 3>& displacement) {
  // move of our mass
  SetMassLocation(Math::Add(GetMassLocation(), displacement));
  // Recompute length, tension and re-center the computation node, and
  // redefine axis
  UpdateDependentPhysicalVariables();
  UpdateLocalCoordinateAxis();

  // FIXME this whole block might be superfluous - ApplyDisplacement is called
  // For the relatives: recompute the lenght, tension etc. (why for mother?
  // have to think about that)
  if (daughter_left_ != nullptr) {
    // FIXME this is problematic for the distributed version. it modifies a
    // "neightbor"
    daughter_left_->UpdateDependentPhysicalVariables();
    daughter_left_->UpdateLocalCoordinateAxis();
  }
  if (daughter_right_ != nullptr) {
    // FIXME this is problematic for the distributed version. it modifies a
    // "neightbor"
    daughter_right_->UpdateDependentPhysicalVariables();
    daughter_right_->UpdateLocalCoordinateAxis();
  }
}

void NeuriteElement::UpdateLocalCoordinateAxis() {
  // x (new) = something new
  // z (new) = x (new) cross y(old)
  // y (new) = z(new) cross x(new)
  x_axis_ = Math::Normalize(spring_axis_);
  z_axis_ = Math::CrossProduct(x_axis_, y_axis_);
  double norm_of_z = Math::Norm(z_axis_);
  if (norm_of_z < 1E-10) {  // TODO(neurites) use parameter
    // If new x_axis_ and old y_axis_ are aligned, we cannot use this scheme;
    // we start by re-defining new perp vectors. Ok, we loose the previous
    // info, but this should almost never happen....
    auto* random = Simulation::GetActive()->GetRandom();
    z_axis_ = Math::Perp3(x_axis_, random->Uniform(0, 1));
  } else {
    z_axis_ = Math::ScalarMult((1 / norm_of_z), z_axis_);
  }
  y_axis_ = Math::CrossProduct(z_axis_, x_axis_);
}

void NeuriteElement::UpdateDiameter() {
  diameter_ = std::sqrt(4 / Math::kPi * volume_ / actual_length_);
}

void NeuriteElement::UpdateVolume() {
  volume_ = Math::kPi / 4 * diameter_ * diameter_ * actual_length_;
}

// ***************************************************************************
//   Coordinates transform
// ***************************************************************************

std::array<double, 3> NeuriteElement::TransformCoordinatesGlobalToLocal(
    const std::array<double, 3>& position) const {
  auto pos = Math::Subtract(position, ProximalEnd());
  return {Math::Dot(pos, x_axis_), Math::Dot(pos, y_axis_),
          Math::Dot(pos, z_axis_)};
}

std::array<double, 3> NeuriteElement::TransformCoordinatesLocalToGlobal(
    const std::array<double, 3>& position) const {
  std::array<double, 3> glob{
      position[0] * x_axis_[0] + position[1] * y_axis_[0] +
          position[2] * z_axis_[0],
      position[0] * x_axis_[1] + position[1] * y_axis_[1] +
          position[2] * z_axis_[1],
      position[0] * x_axis_[2] + position[1] * y_axis_[2] +
          position[2] * z_axis_[2]};
  return Math::Add(glob, ProximalEnd());
}

std::array<double, 3> NeuriteElement::TransformCoordinatesLocalToPolar(
    const std::array<double, 3>& position) const {
  return {position[0], std::atan2(position[2], position[1]),
          std::sqrt(position[1] * position[1] + position[2] * position[2])};
}

std::array<double, 3> NeuriteElement::TransformCoordinatesPolarToLocal(
    const std::array<double, 3>& position) const {
  return {position[0], position[2] * std::cos(position[1]),
          position[2] * std::sin(position[1])};
}

std::array<double, 3> NeuriteElement::TransformCoordinatesPolarToGlobal(
    const std::array<double, 2>& position) const {
  // the position is in cylindrical coord (h,theta,r)
  // with r being implicit (half the diameter_)
  // We thus have h (along x_axis_) and theta (the angle from the y_axis_).
  double r = 0.5 * diameter_;
  std::array<double, 3> polar_position{position[0], position[1], r};
  auto local = TransformCoordinatesPolarToLocal(polar_position);
  return TransformCoordinatesLocalToGlobal(local);
}

std::array<double, 3> NeuriteElement::TransformCoordinatesGlobalToPolar(
    const std::array<double, 3>& position) const {
  auto local = TransformCoordinatesGlobalToLocal(position);
  return TransformCoordinatesLocalToPolar(local);
}

// ***************************************************************************
//   GETTERS & SETTERS
// ***************************************************************************

bool NeuriteElement::IsAxon() const { return is_axon_; }

void NeuriteElement::SetAxon(bool is_axon) { is_axon_ = is_axon; }

NeuronOrNeurite* NeuriteElement::GetMother() { return mother_.Get(); }
const NeuronOrNeurite* NeuriteElement::GetMother() const {
  return mother_.Get();
}
void NeuriteElement::SetMother(const SoPointer<NeuronOrNeurite>& mother) {
  mother_ = mother;
}

const SoPointer<NeuriteElement>& NeuriteElement::GetDaughterLeft() const {
  return daughter_left_;
}

void NeuriteElement::SetDaughterLeft(
    const SoPointer<NeuriteElement>& daughter) {
  daughter_left_ = daughter;
}

const SoPointer<NeuriteElement>& NeuriteElement::GetDaughterRight() const {
  return daughter_right_;
}

void NeuriteElement::SetDaughterRight(
    const SoPointer<NeuriteElement>& daughter) {
  daughter_right_ = daughter;
}

int NeuriteElement::GetBranchOrder() const { return branch_order_; }

void NeuriteElement::SetBranchOrder(int branch_order) {
  branch_order_ = branch_order;
}

double NeuriteElement::GetActualLength() const { return actual_length_; }

/// Should not be used, since the actual length depends on the geometry.
void NeuriteElement::SetActualLength(double actual_length) {
  actual_length_ = actual_length;
}

double NeuriteElement::GetRestingLength() const { return resting_length_; }

void NeuriteElement::SetRestingLength(double resting_length) {
  resting_length_ = resting_length;
}

const std::array<double, 3>& NeuriteElement::GetSpringAxis() const {
  return spring_axis_;
}

void NeuriteElement::SetSpringAxis(const std::array<double, 3>& axis) {
  spring_axis_ = axis;
}

double NeuriteElement::GetSpringConstant() const { return spring_constant_; }

void NeuriteElement::SetSpringConstant(double spring_constant) {
  spring_constant_ = spring_constant;
}

double NeuriteElement::GetTension() const { return tension_; }

void NeuriteElement::SetTension(double tension) { tension_ = tension; }

std::array<double, 3> NeuriteElement::GetUnitaryAxisDirectionVector() const {
  double factor = 1.0 / actual_length_;
  return Math::ScalarMult(factor, spring_axis_);
}

bool NeuriteElement::IsTerminal() const { return daughter_left_ == nullptr; }

std::array<double, 3> NeuriteElement::ProximalEnd() const {
  return Math::Subtract(mass_location_, spring_axis_);
}

const std::array<double, 3>& NeuriteElement::DistalEnd() const {
  return mass_location_;
}

double NeuriteElement::LengthToProximalBranchingPoint() const {
  double length = actual_length_;
  if (auto* mother_neurite = mother_->As<NeuriteElement>()) {
    if (mother_neurite->GetDaughterRight() == nullptr) {
      length += mother_neurite->LengthToProximalBranchingPoint();
    }
  }
  return length;
}

double NeuriteElement::GetLength() const { return actual_length_; }

const std::array<double, 3>& NeuriteElement::GetAxis() const {
  // local coordinate x_axis_ is equal to cylinder axis
  return x_axis_;
}

void NeuriteElement::UpdateDependentPhysicalVariables() {
  auto relative_ml = mother_->OriginOf(Base::GetUid());
  spring_axis_ = Math::Subtract(mass_location_, relative_ml);
  actual_length_ = std::sqrt(Math::Dot(spring_axis_, spring_axis_));
  if (std::abs(actual_length_ - resting_length_) > 1e-13) {
    tension_ =
        spring_constant_ * (actual_length_ - resting_length_) / resting_length_;
  } else {
    // avoid floating point rounding effects that increase the tension
    tension_ = 0.0;
  }
  UpdateVolume();
}

std::ostream& operator<<(std::ostream& str, const NeuriteElement& n) {
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
  str << "spring_axis_:     " << n.spring_axis_[0] << ", " << n.spring_axis_[1]
      << ", " << n.spring_axis_[2] << ", " << std::endl;
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
  auto* mother_soma = n.mother_->As<NeuronSoma>();
  auto* mother_neurite = n.mother_->As<NeuriteElement>();
  auto mother =
      mother_soma ? "neuron" : (mother_neurite ? "neurite" : "nullptr");
  str << "mother_           " << mother << std::endl;
  return str;
}

void NeuriteElement::Copy(const NeuriteElement& rhs) {
  // TODO(neurites) adherence
  adherence_ = rhs.GetAdherence();
  //  density_
  SetDiameter(rhs.GetDiameter());  // also updates voluume
  x_axis_ = rhs.GetXAxis();
  y_axis_ = rhs.GetYAxis();
  z_axis_ = rhs.GetZAxis();

  spring_axis_ = rhs.GetSpringAxis();
  branch_order_ = rhs.GetBranchOrder();
  spring_constant_ = rhs.GetSpringConstant();
  // TODO(neurites) what about actual length, tension and resting_length_ ??
}

NeuriteElement* NeuriteElement::SplitNeuriteElement(double distal_portion) {
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  SplitNeuriteElementEvent event(distal_portion);
  auto* new_proximal_element = GetInstance(event, this)->As<NeuriteElement>();
  ctxt->push_back(new_proximal_element);
  EventHandler(event, new_proximal_element);
  return new_proximal_element;
}

void NeuriteElement::RemoveProximalNeuriteElement() {
  // The mother is removed if (a) it is a neurite element and (b) it has no
  // other daughter than
  auto* mother_neurite = mother_->As<NeuriteElement>();
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
  spring_axis_ =
      Math::Subtract(mass_location_, mother_->OriginOf(Base::GetUid()));
  actual_length_ = Math::Norm(spring_axis_);
  resting_length_ =
      spring_constant_ * actual_length_ / (tension_ + spring_constant_);
  // .... and volume_
  UpdateVolume();
  // and local coord
  UpdateLocalCoordinateAxis();

  proximal_ne->RemoveFromSimulation();
}

NeuriteElement* NeuriteElement::ExtendSideNeuriteElement(
    double length, double diameter, const std::array<double, 3>& direction) {
  if (daughter_right_ != nullptr) {
    Fatal("NeuriteElement",
          "Can't extend a side neurite since daughter_right is not a nullptr!");
  }

  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  SideNeuriteExtensionEvent event{length, diameter, direction};
  auto* new_branch = GetInstance(event, this)->As<NeuriteElement>();
  new_branch->EventHandler(event, this);
  ctxt->push_back(new_branch);
  EventHandler(event, new_branch);
  return new_branch;
}

void NeuriteElement::InitializeNewNeuriteExtension(NeuronSoma* soma,
                                                   double diameter, double phi,
                                                   double theta) {
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  tension_ = param->neurite_default_tension_;
  diameter_ = param->neurite_default_diameter_;
  actual_length_ = param->neurite_default_actual_length_;
  density_ = param->neurite_default_density_;
  spring_constant_ = param->neurite_default_spring_constant_;
  adherence_ = param->neurite_default_adherence_;

  double radius = 0.5 * soma->GetDiameter();
  double new_length = param->neurite_default_actual_length_;
  // position in bdm.cells coord
  double x_coord = std::sin(theta) * std::cos(phi);
  double y_coord = std::sin(theta) * std::sin(phi);
  double z_coord = std::cos(theta);
  std::array<double, 3> axis_direction{
      x_coord * soma->kXAxis[0] + y_coord * soma->kYAxis[0] +
          z_coord * soma->kZAxis[0],
      x_coord * soma->kXAxis[1] + y_coord * soma->kYAxis[1] +
          z_coord * soma->kZAxis[1],
      x_coord * soma->kXAxis[2] + y_coord * soma->kYAxis[2] +
          z_coord * soma->kZAxis[2]};

  // positions & axis in cartesian coord
  auto new_begin_location =
      Math::Add(soma->GetPosition(), Math::ScalarMult(radius, axis_direction));
  auto new_spring_axis = Math::ScalarMult(new_length, axis_direction);

  auto new_mass_location = Math::Add(new_begin_location, new_spring_axis);

  // set attributes of new neurite segment
  diameter_ = diameter;
  UpdateVolume();
  spring_axis_ = new_spring_axis;

  SetMassLocation(new_mass_location);
  actual_length_ = new_length;
  SetRestingLengthForDesiredTension(param->neurite_default_tension_);
  UpdateLocalCoordinateAxis();

  // family relations
  SetMother(soma->GetSoPtr<NeuronOrNeurite>());
}

void NeuriteElement::InitializeNeuriteBifurcation(
    NeuriteElement* mother, double length, double diameter,
    const std::array<double, 3>& direction) {
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  tension_ = param->neurite_default_tension_;
  diameter_ = param->neurite_default_diameter_;
  actual_length_ = param->neurite_default_actual_length_;
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
    proj = Math::ScalarMult(-1, proj);
    dir_1 = Math::Add(direction, proj);
  }

  // mass location and spring axis
  const auto& mother_ml = mother->GetMassLocation();
  SetSpringAxis(Math::ScalarMult(length, Math::Normalize(dir_1)));
  SetMassLocation(Math::Add(mother_ml, spring_axis_));
  UpdateLocalCoordinateAxis();  // (important so that x_axis_ is correct)

  // physics of tension :
  actual_length_ = length;
  SetRestingLengthForDesiredTension(param->neurite_default_tension_);

  // set local coordinate axis in the new branches
  // TODO(neurites) again?? alreay done a few lines up
  UpdateLocalCoordinateAxis();

  // 2) creating the first daughter branch
  diameter_ = diameter;
  branch_order_ = mother->GetBranchOrder() + 1;

  UpdateDependentPhysicalVariables();
}

void NeuriteElement::InitializeSplitOrBranching(NeuriteElement* other,
                                                double distal_portion) {
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  tension_ = param->neurite_default_tension_;
  diameter_ = param->neurite_default_diameter_;
  actual_length_ = param->neurite_default_actual_length_;
  density_ = param->neurite_default_density_;
  spring_constant_ = param->neurite_default_spring_constant_;
  adherence_ = param->neurite_default_adherence_;

  const auto& other_ml = other->GetMassLocation();
  const auto& other_sa = other->GetSpringAxis();
  const auto& other_rl = other->GetRestingLength();

  // TODO(neurites) reformulate to mass_location_
  auto new_position =
      Math::Subtract(other_ml, Math::ScalarMult(distal_portion, other_sa));

  SetPosition(new_position);
  Copy(*other);

  // family relations
  SetMother(other->GetMother()->GetNeuronOrNeuriteSoPtr());
  SetDaughterLeft(other->GetSoPtr<NeuriteElement>());

  // physics
  resting_length_ = ((1 - distal_portion) * other_rl);
}

void NeuriteElement::InitializeSideExtensionOrBranching(
    NeuriteElement* mother, double length, double diameter,
    const std::array<double, 3>& direction) {
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  tension_ = param->neurite_default_tension_;
  diameter_ = param->neurite_default_diameter_;
  actual_length_ = param->neurite_default_actual_length_;
  density_ = param->neurite_default_density_;
  spring_constant_ = param->neurite_default_spring_constant_;
  adherence_ = param->neurite_default_adherence_;

  Copy(*mother);

  auto dir = direction;
  const auto& mother_spring_axis = mother->GetSpringAxis();
  double angle_with_side_branch =
      Math::AngleRadian(mother_spring_axis, direction);
  if (angle_with_side_branch < 0.78 ||
      angle_with_side_branch > 2.35) {  // 45-135 degrees
    auto p = Math::CrossProduct(mother_spring_axis, direction);
    p = Math::CrossProduct(p, mother_spring_axis);
    dir = Math::Add(Math::Normalize(direction), Math::Normalize(p));
  }
  // location of mass and computation center
  auto new_spring_axis = Math::ScalarMult(length, Math::Normalize(direction));
  const auto& mother_ml = mother->GetMassLocation();

  SetMassLocation(Math::Add(mother_ml, new_spring_axis));
  SetSpringAxis(new_spring_axis);
  // physics
  SetActualLength(length);
  SetRestingLengthForDesiredTension(param->neurite_default_tension_);
  SetDiameter(param->neurite_default_diameter_);
  UpdateLocalCoordinateAxis();
  // family relations
  SetMother(mother->GetSoPtr<NeuronOrNeurite>());

  branch_order_ = mother->GetBranchOrder() + 1;

  diameter_ = diameter;

  // correct physical values (has to be after family relations
  UpdateDependentPhysicalVariables();
}

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm
