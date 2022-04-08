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

#include "neuroscience/neurite_element.h"
#include <string>

namespace bdm {
namespace neuroscience {

NeuriteElement::NeuriteElement() {
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();
  tension_ = param->neurite_default_tension;
  SetDiameter(param->neurite_default_diameter);
  SetActualLength(param->neurite_default_actual_length);
  density_ = param->neurite_default_density;
  spring_constant_ = param->neurite_default_spring_constant;
  adherence_ = param->neurite_default_adherence;
  resting_length_ =
      spring_constant_ * actual_length_ / (tension_ + spring_constant_);
  UpdateVolume();
}

void NeuriteElement::Initialize(const NewAgentEvent& event) {
  Base::Initialize(event);

  if (event.GetUid() == NewNeuriteExtensionEvent::kUid) {
    const auto& e = static_cast<const NewNeuriteExtensionEvent&>(event);
    auto* soma = bdm_static_cast<NeuronSoma*>(event.existing_agent);
    InitializeNewNeuriteExtension(soma, e.diameter, e.phi, e.theta);
  } else if (event.GetUid() == NeuriteBifurcationEvent::kUid) {
    const auto& e = static_cast<const NeuriteBifurcationEvent&>(event);
    auto* ne = bdm_static_cast<NeuriteElement*>(event.existing_agent);
    double diameter;
    Double3 direction;
    if (event.new_agents.size() == 0) {
      // left branch
      diameter = e.diameter_left;
      direction = e.direction_left;
    } else {
      // right branch
      diameter = e.diameter_right;
      direction = e.direction_right;
    }
    InitializeNeuriteBifurcation(ne, e.length, diameter, direction);
  } else if (event.GetUid() == SideNeuriteExtensionEvent::kUid) {
    const auto& e = static_cast<const SideNeuriteExtensionEvent&>(event);
    auto* ne = bdm_static_cast<NeuriteElement*>(event.existing_agent);
    InitializeSideExtensionOrBranching(ne, e.length, e.diameter, e.direction);
  } else if (event.GetUid() == SplitNeuriteElementEvent::kUid) {
    const auto& e = static_cast<const SplitNeuriteElementEvent&>(event);
    auto* ne = bdm_static_cast<NeuriteElement*>(event.existing_agent);
    InitializeSplitOrBranching(ne, e.distal_portion);
  } else if (event.GetUid() == NeuriteBranchingEvent::kUid) {
    const auto& e = static_cast<const NeuriteBranchingEvent&>(event);
    if (event.new_agents.size() == 0) {
      auto* ne = bdm_static_cast<NeuriteElement*>(event.existing_agent);
      InitializeSplitOrBranching(ne, e.distal_portion);
    } else {
      // new proximal neurite element
      auto* ne = bdm_static_cast<NeuriteElement*>(event.new_agents[0]);
      InitializeSideExtensionOrBranching(ne, e.length, e.diameter, e.direction);
    }
  }
}

void NeuriteElement::Update(const NewAgentEvent& event) {
  Base::Update(event);

  auto new_agent1 = event.new_agents[0];
  auto new_agent2 = event.new_agents[1];

  if (event.GetUid() == NeuriteBifurcationEvent::kUid) {
    SetDaughterLeft(new_agent1->GetAgentPtr<NeuriteElement>());
    SetDaughterRight(new_agent2->GetAgentPtr<NeuriteElement>());
  } else if (event.GetUid() == SideNeuriteExtensionEvent::kUid) {
    SetDaughterRight(new_agent2->GetAgentPtr<NeuriteElement>());
  } else if (event.GetUid() == SplitNeuriteElementEvent::kUid) {
    const auto& e = static_cast<const SplitNeuriteElementEvent&>(event);
    auto* proximal = bdm_static_cast<NeuriteElement*>(new_agent1);
    resting_length_ *= e.distal_portion;

    // family relations
    mother_->UpdateRelative(*this, *proximal);
    mother_ = proximal->GetAgentPtr<NeuronOrNeurite>();

    UpdateDependentPhysicalVariables();
    proximal->UpdateDependentPhysicalVariables();
    // UpdateLocalCoordinateAxis has to come after UpdateDepend...
    proximal->UpdateLocalCoordinateAxis();
  } else if (event.GetUid() == NeuriteBranchingEvent::kUid) {
    const auto& e = static_cast<const NeuriteBranchingEvent&>(event);
    auto* proximal = bdm_static_cast<NeuriteElement*>(new_agent1);
    auto* branch = bdm_static_cast<NeuriteElement*>(new_agent2);

    // TODO(lukas) some code duplication with SplitNeuriteElementEvent and
    // SideNeuriteExtensionEvent event handler
    proximal->SetDaughterRight(branch->GetAgentPtr<NeuriteElement>());

    // elongation
    resting_length_ *= e.distal_portion;
    mother_->UpdateRelative(*this, *proximal);
    mother_ = proximal->GetAgentPtr<NeuronOrNeurite>();

    UpdateDependentPhysicalVariables();
    proximal->UpdateDependentPhysicalVariables();
    // UpdateLocalCoordinateAxis has to come after UpdateDepend...
    proximal->UpdateLocalCoordinateAxis();
  }
}

void NeuriteElement::CriticalRegion(std::vector<AgentPointer<>>* aptrs) {
  aptrs->reserve(4);
  aptrs->push_back(GetAgentPtr<>());
  aptrs->push_back(mother_);
  aptrs->push_back(daughter_left_);
  aptrs->push_back(daughter_right_);
}

std::set<std::string> NeuriteElement::GetRequiredVisDataMembers() const {
  return {"mass_location_", "diameter_", "actual_length_", "spring_axis_"};
}

void NeuriteElement::SetDiameter(double diameter) {
  if (diameter > diameter_) {
    SetPropagateStaticness();
  }
  diameter_ = diameter;
  UpdateVolume();
}

void NeuriteElement::SetDensity(double density) {
  if (density > density_) {
    SetPropagateStaticness();
  }
  density_ = density;
}

void NeuriteElement::SetPosition(const Double3& position) {
  position_ = position;
  SetMassLocation(position + spring_axis_ * 0.5);
}

void NeuriteElement::UpdatePosition() {
  position_ = mass_location_ - (spring_axis_ * 0.5);
  SetPropagateStaticness();
}

void NeuriteElement::SetMassLocation(const Double3& mass_location) {
  mass_location_ = mass_location;
  SetPropagateStaticness();
}

void NeuriteElement::SetAdherence(double adherence) {
  if (adherence < adherence_) {
    SetStaticnessNextTimestep(false);
  }
  adherence_ = adherence;
}

Double3 NeuriteElement::OriginOf(const AgentUid& daughter_uid) const {
  return mass_location_;
}

StructureIdentifierSWC NeuriteElement::GetIdentifierSWC() const {
  if (IsAxon()) {
    return StructureIdentifierSWC::kAxon;
  } else {
    return StructureIdentifierSWC::kApicalDendrite;
  }
};

void NeuriteElement::RetractTerminalEnd(double speed) {
  // check if is a terminal branch
  if (daughter_left_ != nullptr) {
    return;
  }
  // scaling for integration step
  auto* core_param = Simulation::GetActive()->GetParam();
  speed *= core_param->simulation_time_step;

  auto* mother_agentma = dynamic_cast<NeuronSoma*>(mother_.Get());
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
  } else if (mother_agentma) {
    mother_->RemoveDaughter(Base::GetAgentPtr<NeuriteElement>());
    this->RemoveFromSimulation();
  } else if (mother_neurite && mother_neurite->GetDaughterRight() == nullptr) {
    // if actual_length_ < length and mother is a neurite element with no
    // other daughter : merge with mother
    RemoveProximalNeuriteElement();  // also updates volume_...
    RetractTerminalEnd(speed / core_param->simulation_time_step);
  } else {
    // if mother is neurite element with other daughter or is not a neurite
    // segment: disappear.
    mother_->RemoveDaughter(Base::GetAgentPtr<NeuriteElement>());
    this->RemoveFromSimulation();

    mother_->UpdateDependentPhysicalVariables();
  }
}

void NeuriteElement::ElongateTerminalEnd(double speed,
                                         const Double3& direction) {
  double temp = direction * spring_axis_;
  if (temp > 0) {
    MovePointMass(speed, direction);
  }
}

bool NeuriteElement::BranchPermitted() const {
  return daughter_left_ != nullptr && daughter_right_ == nullptr;
}

NeuriteElement* NeuriteElement::Branch(double new_branch_diameter,
                                       const Double3& direction,
                                       double length) {
  // create a new neurite element for side branch
  // we first split this neurite element into two pieces
  // then append a "daughter right" between the two
  NeuriteBranchingEvent event(0.5, length, new_branch_diameter, direction);
  CreateNewAgents(event, {this, this});
  return bdm_static_cast<NeuriteElement*>(event.new_agents[1]);
}

NeuriteElement* NeuriteElement::Branch(const Double3& direction) {
  return Branch(diameter_, direction);
}

NeuriteElement* NeuriteElement::Branch(double diameter) {
  auto* random = Simulation::GetActive()->GetRandom();
  auto rand_noise = random->template UniformArray<3>(-0.1, 0.1);
  auto growth_direction = Math::Perp3(
      GetUnitaryAxisDirectionVector() + rand_noise, random->Uniform(0, 1));
  growth_direction.Normalize();
  return Branch(diameter, growth_direction);
}

NeuriteElement* NeuriteElement::Branch() {
  auto* random = Simulation::GetActive()->GetRandom();
  double branch_diameter = diameter_;
  auto rand_noise = random->template UniformArray<3>(-0.1, 0.1);
  auto growth_direction = Math::Perp3(
      GetUnitaryAxisDirectionVector() + rand_noise, random->Uniform(0, 1));
  return Branch(branch_diameter, growth_direction);
}

bool NeuriteElement::BifurcationPermitted() const {
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();
  return (daughter_left_ == nullptr &&
          actual_length_ > param->neurite_minimial_bifurcation_length);
}

std::array<NeuriteElement*, 2> NeuriteElement::Bifurcate(
    double length, double diameter_1, double diameter_2,
    const Double3& direction_1, const Double3& direction_2) {
  // 1) physical bifurcation
  // check it is a terminal branch
  if (daughter_left_ != nullptr) {
    Fatal("NeuriteElements",
          "Bifurcation only allowed on a terminal neurite element");
  }
  NeuriteBifurcationEvent event(length, diameter_1, diameter_2, direction_1,
                                direction_2);
  CreateNewAgents(event, {this, this});
  auto* new_branch_l = bdm_static_cast<NeuriteElement*>(event.new_agents[0]);
  auto* new_branch_r = bdm_static_cast<NeuriteElement*>(event.new_agents[1]);
  return {new_branch_l, new_branch_r};
}

std::array<NeuriteElement*, 2> NeuriteElement::Bifurcate(
    double diameter_1, double diameter_2, const Double3& direction_1,
    const Double3& direction_2) {
  Log::Fatal("NeuriteElement::Bifurcate", "Not Implemented");
  std::array<NeuriteElement*, 2> dummy;
  return dummy;
}

std::array<NeuriteElement*, 2> NeuriteElement::Bifurcate(
    const Double3& direction_1, const Double3& direction_2) {
  // initial default length :
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();
  double l = param->neurite_default_actual_length;
  // diameters :
  double d = diameter_;
  return Bifurcate(l, d, d, direction_1, direction_2);
}

std::array<NeuriteElement*, 2> NeuriteElement::Bifurcate() {
  // initial default length :
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();
  double l = param->neurite_default_actual_length;
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

void NeuriteElement::RemoveDaughter(
    const AgentPointer<NeuriteElement>& daughter) {
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
  if (mother_ == &old_relative) {
    mother_ = new_relative.GetNeuronOrNeuriteAgentPtr();
  } else {
    auto new_neurite_agent_ptr =
        bdm_static_cast<const NeuriteElement*>(&new_relative)
            ->GetAgentPtr<NeuriteElement>();
    if (daughter_left_ == &old_relative) {
      daughter_left_ = new_neurite_agent_ptr;
    } else if (daughter_right_ == &old_relative) {
      daughter_right_ = new_neurite_agent_ptr;
    }
  }
}

Double3 NeuriteElement::ForceTransmittedFromDaugtherToMother(
    const NeuronOrNeurite& mother) {
  if (mother_ != &mother) {
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

void NeuriteElement::RunDiscretization() {
  if (daughter_left_ != nullptr) {
    return;
  }

  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();
  auto* mother_agentma = dynamic_cast<NeuronSoma*>(mother_.Get());
  auto* mother_neurite = dynamic_cast<NeuriteElement*>(mother_.Get());
  if (actual_length_ > param->neurite_max_length) {
    if (daughter_left_ == nullptr) {  // if terminal branch :
      SplitNeuriteElement(0.1);
    } else if (mother_agentma != nullptr) {  // if initial branch :
      SplitNeuriteElement(0.9);
    } else {
      SplitNeuriteElement(0.5);
    }
  } else if (actual_length_ < param->neurite_min_length &&
             mother_neurite != nullptr &&
             mother_neurite->GetRestingLength() <
                 param->neurite_max_length - resting_length_ - 1 &&
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

void NeuriteElement::MovePointMass(double speed, const Double3& direction) {
  // check if is a terminal branch
  if (daughter_left_ != nullptr) {
    return;
  }

  // scaling for integration step
  auto* core_param = Simulation::GetActive()->GetParam();
  double length = speed * core_param->simulation_time_step;
  auto displacement = direction.GetNormalizedArray() * length;
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

void NeuriteElement::SetRestingLengthForDesiredTension(double tension) {
  SetTension(tension);
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
  double delta = speed * core_param->simulation_time_step;
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
  double delta = speed * core_param->simulation_time_step;
  diameter_ += delta;
  UpdateVolume();
}

void NeuriteElement::MechanicalForcesFunctor::operator()(
    Agent* neighbor, double squared_distance) {
  // if neighbor is a NeuriteElement
  // use shape to determine if neighbor is a NeuriteElement
  // this is much faster than using a dynamic_cast
  if (neighbor->GetShape() == Shape::kCylinder) {
    // if it is a direct relative, or sister branch, we don't take it into
    // account
    if (ne->GetDaughterLeft() == neighbor ||
        ne->GetDaughterRight() == neighbor ||
        ne->GetMother() ==
            bdm_static_cast<const NeuriteElement*>(neighbor)->GetMother() ||
        (ne->GetMother() == neighbor)) {
      return;
    }
  } else if (auto* neighbor_soma = dynamic_cast<const NeuronSoma*>(neighbor)) {
    // if neighbor is NeuronSoma
    // if it is a direct relative, we don't take it into account
    if (ne->GetMother() == neighbor_soma) {
      return;
    }
  }

  Double4 force_from_neighbor = force->Calculate(ne, neighbor);

  // hack: if the neighbour is a neurite, we need to reduce the force from
  // that neighbour in order to avoid kink behaviour
  if (neighbor->GetShape() == Shape::kCylinder) {
    force_from_neighbor = force_from_neighbor * h_over_m;
    has_neurite_neighbor = true;
  }

  if (force_from_neighbor[0] != 0 || force_from_neighbor[1] != 0 ||
      force_from_neighbor[2] != 0) {
    non_zero_neighbor_force++;
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
}

Double3 NeuriteElement::CalculateDisplacement(const InteractionForce* force,
                                              double squared_radius,
                                              double dt) {
  Double3 force_on_my_mothers_point_mass{0, 0, 0};

  // 1) Spring force
  //   Only the spring of this cylinder. The daughters spring also act on this
  //    mass, but they are treated in point (2)
  double factor = -tension_ / actual_length_;  // the minus sign is important
                                               // because the spring axis goes
                                               // in the opposite direction
  Double3 force_on_my_point_mass = spring_axis_ * factor;

  // 2) InteractionForce transmitted by daugthers (if they exist)
  if (daughter_left_ != nullptr) {
    force_on_my_point_mass +=
        daughter_left_->ForceTransmittedFromDaugtherToMother(*this);
  }
  if (daughter_right_ != nullptr) {
    force_on_my_point_mass +=
        daughter_right_->ForceTransmittedFromDaugtherToMother(*this);
  }

  Double3 force_from_neighbors = {0, 0, 0};

  auto* core_param = Simulation::GetActive()->GetParam();
  // this value will be used to reduce force for neurite/neurite interactions
  double h_over_m = 0.01;

  // 3) Object avoidance force
  uint64_t non_zero_neighbor_force = 0;
  //  (We check for every neighbor object if they touch us, i.e. push us away)
  if (!IsStatic()) {
    has_neurite_neighbor_ = false;
    MechanicalForcesFunctor calculate_neighbor_forces(
        force, this, force_from_neighbors, force_on_my_mothers_point_mass,
        h_over_m, has_neurite_neighbor_, non_zero_neighbor_force);
    auto* ctxt = Simulation::GetActive()->GetExecutionContext();
    ctxt->ForEachNeighbor(calculate_neighbor_forces, *this, squared_radius);

    if (non_zero_neighbor_force > 1) {
      SetStaticnessNextTimestep(false);
    }
  }

  // hack: if the neighbour is a neurite, and as we reduced the force from
  // that neighbour, we also need to reduce my internal force (from internal
  // tension and daughters)
  if (has_neurite_neighbor_) {
    force_on_my_point_mass *= h_over_m;
  }

  force_on_my_point_mass += force_from_neighbors;

  // 5) define the force that will be transmitted to the mother
  force_to_transmit_to_proximal_mass_ = force_on_my_mothers_point_mass;
  if (!IsStatic() && force_to_transmit_to_proximal_mass_ != Double3{0, 0, 0}) {
    if (mother_->IsNeuriteElement()) {
      bdm_static_cast<NeuriteElement*>(mother_.Get())
          ->SetStaticnessNextTimestep(false);
    } else {
      bdm_static_cast<NeuronSoma*>(mother_.Get())
          ->SetStaticnessNextTimestep(false);
    }
  }
  //  6.1) Define movement scale

  //  6.2) If is F not strong enough -> no movements
  if (force_on_my_point_mass == Double3{0, 0, 0}) {
    return {0, 0, 0};
  }

  double force_norm = force_on_my_point_mass.Norm();
  if (force_norm < adherence_) {
    return {0, 0, 0};
  }

  // So, what follows is only executed if we do actually move :

  //  6.3) Since there's going be a move, we calculate it
  auto& displacement = force_on_my_point_mass;
  double& displacement_norm = force_norm;

  //  6.4) There is an upper bound for the movement.
  if (displacement_norm > core_param->simulation_max_displacement) {
    displacement = displacement * (core_param->simulation_max_displacement /
                                   displacement_norm);
  }

  return displacement;
}

void NeuriteElement::ApplyDisplacement(const Double3& displacement) {
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

void NeuriteElement::UpdateLocalCoordinateAxis() {
  // x (new) = something new
  // z (new) = x (new) cross y(old)
  // y (new) = z(new) cross x(new)
  x_axis_ = spring_axis_.GetNormalizedArray();
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

void NeuriteElement::UpdateDiameter() {
  double diameter = std::sqrt(4 / Math::kPi * volume_ / actual_length_);
  if (diameter > diameter_) {
    Base::SetPropagateStaticness();
  }
  diameter_ = diameter;
}

void NeuriteElement::UpdateVolume() {
  volume_ = Math::kPi / 4 * diameter_ * diameter_ * actual_length_;
}

Double3 NeuriteElement::TransformCoordinatesGlobalToLocal(
    const Double3& position) const {
  auto pos = position - ProximalEnd();
  return {pos * x_axis_, pos * y_axis_, pos * z_axis_};
}

Double3 NeuriteElement::TransformCoordinatesLocalToGlobal(
    const Double3& position) const {
  Double3 axis_0{x_axis_[0], y_axis_[0], z_axis_[0]};
  Double3 axis_1{x_axis_[1], y_axis_[1], z_axis_[1]};
  Double3 axis_2{x_axis_[2], y_axis_[2], z_axis_[2]};
  auto x = position * axis_0;
  auto y = position * axis_1;
  auto z = position * axis_2;
  Double3 glob{x, y, z};
  return glob + ProximalEnd();
}

Double3 NeuriteElement::TransformCoordinatesLocalToPolar(
    const Double3& position) const {
  return {position[0], std::atan2(position[2], position[1]),
          std::sqrt(position[1] * position[1] + position[2] * position[2])};
}

Double3 NeuriteElement::TransformCoordinatesPolarToLocal(
    const Double3& position) const {
  return {position[0], position[2] * std::cos(position[1]),
          position[2] * std::sin(position[1])};
}

Double3 NeuriteElement::TransformCoordinatesPolarToGlobal(
    const std::array<double, 2>& position) const {
  // the position is in cylindrical coord (h,theta,r)
  // with r being implicit (half the diameter_)
  // We thus have h (along x_axis_) and theta (the angle from the y_axis_).
  double r = 0.5 * diameter_;
  Double3 polar_position{position[0], position[1], r};
  auto local = TransformCoordinatesPolarToLocal(polar_position);
  return TransformCoordinatesLocalToGlobal(local);
}

Double3 NeuriteElement::TransformCoordinatesGlobalToPolar(
    const Double3& position) const {
  auto local = TransformCoordinatesGlobalToLocal(position);
  return TransformCoordinatesLocalToPolar(local);
}

const AgentPointer<NeuriteElement>& NeuriteElement::GetDaughterLeft() const {
  return daughter_left_;
}

void NeuriteElement::SetDaughterLeft(
    const AgentPointer<NeuriteElement>& daughter) {
  daughter_left_ = daughter;
}

const AgentPointer<NeuriteElement>& NeuriteElement::GetDaughterRight() const {
  return daughter_right_;
}

void NeuriteElement::SetDaughterRight(
    const AgentPointer<NeuriteElement>& daughter) {
  daughter_right_ = daughter;
}

void NeuriteElement::SetActualLength(double actual_length) {
  if (actual_length > actual_length_) {
    SetPropagateStaticness();
  }
  actual_length_ = actual_length;
}

void NeuriteElement::SetRestingLength(double resting_length) {
  resting_length_ = resting_length;
}

void NeuriteElement::SetSpringAxis(const Double3& axis) {
  SetPropagateStaticness();
  spring_axis_ = axis;
}

void NeuriteElement::SetSpringConstant(double spring_constant) {
  spring_constant_ = spring_constant;
}

void NeuriteElement::SetTension(double tension) {
  if (tension > tension_) {
    SetStaticnessNextTimestep(false);
  }
  tension_ = tension;
}

Double3 NeuriteElement::GetUnitaryAxisDirectionVector() const {
  double factor = 1.0 / actual_length_;
  return spring_axis_ * factor;
}

double NeuriteElement::LengthToProximalBranchingPoint() const {
  double length = actual_length_;
  if (auto* mother_neurite =
          dynamic_cast<const NeuriteElement*>(mother_.Get())) {
    if (mother_neurite->GetDaughterRight() == nullptr) {
      length += mother_neurite->LengthToProximalBranchingPoint();
    }
  }
  return length;
}

const Double3& NeuriteElement::GetAxis() const {
  // local coordinate x_axis_ is equal to cylinder axis
  return x_axis_;
}

void NeuriteElement::UpdateDependentPhysicalVariables() {
  auto relative_ml = mother_->OriginOf(Base::GetUid());
  SetSpringAxis(mass_location_ - relative_ml);
  SetActualLength(std::sqrt(spring_axis_ * spring_axis_));
  UpdatePosition();
  if (std::abs(actual_length_ - resting_length_) > 1e-13) {
    SetTension(spring_constant_ * (actual_length_ - resting_length_) /
               resting_length_);
  } else {
    // avoid floating point rounding effects that increase the tension
    SetTension(0.0);
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
  auto* mother_agentma = dynamic_cast<const NeuronSoma*>(n.mother_.Get());
  auto* mother_neurite = dynamic_cast<const NeuriteElement*>(n.mother_.Get());
  std::string mother;
  if (mother_agentma) {
    mother = "neuron";
  } else if (mother_neurite) {
    mother = "neurite";
  } else {
    mother = "nullptr";
  }
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

  SetSpringAxis(rhs.GetSpringAxis());
  branch_order_ = rhs.GetBranchOrder();
  spring_constant_ = rhs.GetSpringConstant();
  // TODO(neurites) what about actual length, tension and resting_length_ ??
}

NeuriteElement* NeuriteElement::SplitNeuriteElement(double distal_portion) {
  SplitNeuriteElementEvent event(distal_portion);
  CreateNewAgents(event, {this});
  return bdm_static_cast<NeuriteElement*>(event.new_agents[0]);
}

void NeuriteElement::RemoveProximalNeuriteElement() {
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
  SetMother(mother_neurite->GetMother()->GetNeuronOrNeuriteAgentPtr());

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

NeuriteElement* NeuriteElement::ExtendSideNeuriteElement(
    double length, double diameter, const Double3& direction) {
  if (daughter_right_ != nullptr) {
    Fatal("NeuriteElement",
          "Can't extend a side neurite since daughter_right is not a nullptr!");
  }

  SideNeuriteExtensionEvent event{length, diameter, direction};
  CreateNewAgents(event, {this});
  return bdm_static_cast<NeuriteElement*>(event.new_agents[0]);
}

void NeuriteElement::InitializeNewNeuriteExtension(NeuronSoma* soma,
                                                   double diameter, double phi,
                                                   double theta) {
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();

  double radius = 0.5 * soma->GetDiameter();
  double new_length = param->neurite_default_actual_length;
  // position in bdm.cells coord
  double x_coord = std::sin(theta) * std::cos(phi);
  double y_coord = std::sin(theta) * std::sin(phi);
  double z_coord = std::cos(theta);
  Double3 axis_direction{x_coord * soma->kXAxis[0] + y_coord * soma->kYAxis[0] +
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
  SetRestingLengthForDesiredTension(param->neurite_default_tension);
  UpdateLocalCoordinateAxis();

  // family relations
  SetMother(soma->GetAgentPtr<NeuronOrNeurite>());
}

void NeuriteElement::InitializeNeuriteBifurcation(NeuriteElement* mother,
                                                  double length,
                                                  double diameter,
                                                  const Double3& direction) {
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();

  Copy(*mother);
  SetMother(mother->GetAgentPtr<NeuronOrNeurite>());

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
  SetRestingLengthForDesiredTension(param->neurite_default_tension);

  // set local coordinate axis in the new branches
  // TODO(neurites) again?? alreay done a few lines up
  UpdateLocalCoordinateAxis();

  // 2) creating the first daughter branch
  SetDiameter(diameter);
  branch_order_ = mother->GetBranchOrder() + 1;

  UpdateDependentPhysicalVariables();
}

void NeuriteElement::InitializeSplitOrBranching(NeuriteElement* other,
                                                double distal_portion) {
  const auto& other_ml = other->GetMassLocation();
  const auto& other_sa = other->GetSpringAxis();
  const auto& other_rl = other->GetRestingLength();

  // TODO(neurites) reformulate to mass_location_
  auto new_position = other_ml - (other_sa * distal_portion);

  SetPosition(new_position);
  Copy(*other);
  UpdatePosition();

  // family relations
  SetMother(other->GetMother()->GetNeuronOrNeuriteAgentPtr());
  SetDaughterLeft(other->GetAgentPtr<NeuriteElement>());

  // physics
  resting_length_ = ((1 - distal_portion) * other_rl);
}

void NeuriteElement::InitializeSideExtensionOrBranching(
    NeuriteElement* mother, double length, double diameter,
    const Double3& direction) {
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();

  Copy(*mother);

  auto dir = direction;
  auto direction_normalized = direction.GetNormalizedArray();
  const auto& mother_spring_axis = mother->GetSpringAxis();
  double angle_with_side_branch =
      Math::AngleRadian(mother_spring_axis, direction);
  if (angle_with_side_branch < 0.78 ||
      angle_with_side_branch > 2.35) {  // 45-135 degrees
    auto p = Math::CrossProduct(mother_spring_axis, direction);
    p = Math::CrossProduct(p, mother_spring_axis);
    p.Normalize();
    dir = direction_normalized + p;
  }
  // location of mass and computation center
  auto new_spring_axis = direction_normalized * length;
  const auto& mother_ml = mother->GetMassLocation();

  SetSpringAxis(new_spring_axis);
  SetMassLocation(mother_ml + new_spring_axis);
  UpdatePosition();
  // physics
  SetActualLength(length);
  SetRestingLengthForDesiredTension(param->neurite_default_tension);
  SetDiameter(param->neurite_default_diameter);
  UpdateLocalCoordinateAxis();
  // family relations
  SetMother(mother->GetAgentPtr<NeuronOrNeurite>());

  branch_order_ = mother->GetBranchOrder() + 1;

  SetDiameter(diameter);

  // correct physical values (has to be after family relations
  UpdateDependentPhysicalVariables();
}

}  // namespace neuroscience
}  // namespace bdm
