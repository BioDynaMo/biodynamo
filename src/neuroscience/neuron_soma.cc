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

#include "neuroscience/neuron_soma.h"

#include <algorithm>

#include "core/resource_manager.h"
#include "neuroscience/event/new_neurite_extension_event.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/param.h"

namespace bdm {
namespace neuroscience {

NeuronSoma::NeuronSoma() {}

NeuronSoma::~NeuronSoma() {}

NeuronSoma::NeuronSoma(const Double3& position) : Base(position) {}

NeuronSoma::NeuronSoma(const Event& event, Agent* mother_agent,
                       uint64_t new_oid)
    : Base(event, mother_agent, new_oid) {
  const CellDivisionEvent* cdevent =
      dynamic_cast<const CellDivisionEvent*>(&event);
  NeuronSoma* mother = dynamic_cast<NeuronSoma*>(mother_agent);
  if (cdevent && mother) {
    if (mother->daughters_.size() != 0) {
      Fatal("NeuronSoma",
            "Dividing a neuron soma with attached neurites is not supported "
            "in the default implementation! If you want to change this "
            "behavior derive from this class and overwrite this constructor.");
    }
  }
}

void NeuronSoma::EventHandler(const Event& event, Agent* other1,
                              Agent* other2) {
  Base::EventHandler(event, other1, other2);

  const NewNeuriteExtensionEvent* ne_event =
      dynamic_cast<const NewNeuriteExtensionEvent*>(&event);
  NeuriteElement* neurite = dynamic_cast<NeuriteElement*>(other1);
  if (ne_event && neurite) {
    double theta = ne_event->theta_;
    double phi = ne_event->phi_;
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
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  return ExtendNewNeurite(param->neurite_default_diameter_, angles[2],
                          angles[1], prototype);
}

NeuriteElement* NeuronSoma::ExtendNewNeurite(double diameter, double phi,
                                             double theta,
                                             NeuriteElement* prototype) {
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  NewNeuriteExtensionEvent event(diameter, phi, theta);
  Agent* neurite = nullptr;
  if (!prototype) {
    NeuriteElement ne_tmp;
    neurite = ne_tmp.GetInstance(event, this);
  } else {
    neurite = prototype->GetInstance(event, this);
  }
  ctxt->push_back(neurite);
  EventHandler(event, neurite);
  return bdm_static_cast<NeuriteElement*>(neurite);
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
  auto it =
      std::find(std::begin(daughters_), std::end(daughters_), old_rel_agent_ptr);
  assert(it != std::end(daughters_) &&
         "old_element_idx could not be found in daughters_ vector");
  *it = new_rel_agent_ptr;
  daughters_coord_[new_rel_agent_ptr->GetUid()] = coord;
}

const std::vector<AgentPointer<NeuriteElement>>& NeuronSoma::GetDaughters() const {
  return daughters_;
}

void NeuronSoma::CriticalRegion(std::vector<Spinlock*>* locks) {
  locks->reserve(daughters_.size() + 1);
  locks->push_back(Agent::GetLock());
  for (auto& daughter : daughters_) {
    locks->push_back(daughter->GetLock());
  }
}

}  // namespace neuroscience
}  // namespace bdm
