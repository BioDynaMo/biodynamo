// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include "neuroscience/neuron_soma.h"

#include <algorithm>

#include "core/resource_manager.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/new_agent_event/new_neurite_extension_event.h"
#include "neuroscience/param.h"

namespace bdm {
namespace neuroscience {

NeuronSoma::NeuronSoma() {}

NeuronSoma::~NeuronSoma() {}

NeuronSoma::NeuronSoma(const Double3& position) : Base(position) {}

void NeuronSoma::Initialize(const NewAgentEvent& event) {
  Base::Initialize(event);

  if (event.GetUid() == CellDivisionEvent::kUid) {
    auto* mother = bdm_static_cast<NeuronSoma*>(event.existing_agent);
    if (mother->daughters_.size() != 0) {
      Fatal("NeuronSoma",
            "Dividing a neuron soma with attached neurites is not supported "
            "in the default implementation! If you want to change this "
            "behavior derive from this class and overwrite this method.");
    }
  }
}

void NeuronSoma::Update(const NewAgentEvent& event) {
  Base::Update(event);

  if (event.GetUid() == NewNeuriteExtensionEvent::kUid) {
    const auto& ne_event = static_cast<const NewNeuriteExtensionEvent&>(event);
    auto* neurite = bdm_static_cast<NeuriteElement*>(event.new_agents[0]);

    double theta = ne_event.theta;
    double phi = ne_event.phi;
    double x_coord = std::sin(theta) * std::cos(phi);
    double y_coord = std::sin(theta) * std::sin(phi);
    double z_coord = std::cos(theta);

    daughters_.push_back(neurite->GetAgentPtr<NeuriteElement>());
    daughters_coord_[neurite->GetUid()] = {x_coord, y_coord, z_coord};
  }

  // do nothing for CellDivisionEvent or others
}

NeuriteElement* NeuronSoma::ExtendNewNeurite(const Double3& direction,
                                             NeuriteElement* prototype) {
  auto dir = direction + GetPosition();
  auto angles = Base::TransformCoordinatesGlobalToPolar(dir);
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();
  return ExtendNewNeurite(param->neurite_default_diameter, angles[2], angles[1],
                          prototype);
}

NeuriteElement* NeuronSoma::ExtendNewNeurite(double diameter, double phi,
                                             double theta,
                                             NeuriteElement* prototype) {
  if (!prototype) {
    static NeuriteElement kDefaultNeurite;
    prototype = &kDefaultNeurite;
  }
  NewNeuriteExtensionEvent event(diameter, phi, theta);
  CreateNewAgents(event, {prototype});
  return bdm_static_cast<NeuriteElement*>(event.new_agents[0]);
}

void NeuronSoma::RemoveDaughter(const AgentPointer<NeuriteElement>& daughter) {
  auto it = std::find(std::begin(daughters_), std::end(daughters_), daughter);
  assert(it != std::end(daughters_) &&
         "The element you wanted to remove is not part of daughters_");
  daughters_.erase(it);
}

Double3 NeuronSoma::OriginOf(const AgentUid& daughter_uid) const {
  Double3 xyz = daughters_coord_.at(daughter_uid);

  double radius = GetDiameter() * .5;
  xyz = xyz * radius;

  Double3 axis_0 = {Base::kXAxis[0], Base::kYAxis[0], Base::kZAxis[0]};
  Double3 axis_1 = {Base::kXAxis[1], Base::kYAxis[1], Base::kZAxis[1]};
  Double3 axis_2 = {Base::kXAxis[2], Base::kYAxis[2], Base::kZAxis[2]};

  Double3 result = {xyz * axis_0, xyz * axis_1, xyz * axis_2};
  return GetPosition() + result;
}

void NeuronSoma::UpdateDependentPhysicalVariables() {}

void NeuronSoma::UpdateRelative(const NeuronOrNeurite& old_rel,
                                const NeuronOrNeurite& new_rel) {
  auto old_rel_agent_ptr = bdm_static_cast<const NeuriteElement*>(&old_rel)
                               ->GetAgentPtr<NeuriteElement>();
  auto new_rel_agent_ptr = bdm_static_cast<const NeuriteElement*>(&new_rel)
                               ->GetAgentPtr<NeuriteElement>();
  auto coord = daughters_coord_[old_rel_agent_ptr->GetUid()];
  auto it = std::find(std::begin(daughters_), std::end(daughters_),
                      old_rel_agent_ptr);
  assert(it != std::end(daughters_) &&
         "old_element_idx could not be found in daughters_ vector");
  *it = new_rel_agent_ptr;
  daughters_coord_[new_rel_agent_ptr->GetUid()] = coord;
}

const std::vector<AgentPointer<NeuriteElement>>& NeuronSoma::GetDaughters()
    const {
  return daughters_;
}

void NeuronSoma::CriticalRegion(std::vector<AgentUid>* uids) {
  uids->reserve(daughters_.size() + 1);
  uids->push_back(Agent::GetUid());
  for (auto& daughter : daughters_) {
    uids->push_back(daughter.GetUid());
  }
}

}  // namespace neuroscience
}  // namespace bdm
