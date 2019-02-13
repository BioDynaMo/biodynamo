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
#include "core/event/cell_division_event.h"
#include "core/resource_manager.h"
#include "neuroscience/event/new_neurite_extension_event.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/param.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

NeuronSoma::NeuronSoma() {}

NeuronSoma::~NeuronSoma() {}

NeuronSoma::NeuronSoma(const std::array<double, 3>& position)
    : Base(position) {}

NeuronSoma::NeuronSoma(const Event& event, SimObject* mother_so,
                       uint64_t new_oid)
    : Base(event, mother_so, new_oid) {
  const CellDivisionEvent* cdevent =
      dynamic_cast<const CellDivisionEvent*>(&event);
  NeuronSoma* mother = dynamic_cast<NeuronSoma*>(mother_so);
  if (cdevent && mother) {
    if (mother->daughters_.size() != 0) {
      Fatal("NeuronSoma",
            "Dividing a neuron soma with attached neurites is not supported "
            "in the default implementation! If you want to change this "
            "behavior derive from this class and overwrite this constructor.");
    }
  }
}

void NeuronSoma::EventHandler(const Event& event, SimObject* other1,
                              SimObject* other2) {
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

    daughters_.push_back(neurite->GetSoPtr<NeuriteElement>());
    daughters_coord_[neurite->GetUid()] = {x_coord, y_coord, z_coord};
  }

  // do nothing for CellDivisionEvent or others
}

NeuriteElement* NeuronSoma::ExtendNewNeurite(
    const std::array<double, 3>& direction) {
  auto dir = Math::Add(direction, Base::position_);
  auto angles = Base::TransformCoordinatesGlobalToPolar(dir);
  auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
  return ExtendNewNeurite(param->neurite_default_diameter_, angles[2],
                          angles[1]);
}

NeuriteElement* NeuronSoma::ExtendNewNeurite(double diameter, double phi,
                                             double theta) {
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();
  NewNeuriteExtensionEvent event(diameter, phi, theta);
  NeuriteElement* neurite = new NeuriteElement(event, this);
  ctxt->push_back(neurite);
  EventHandler(event, neurite);
  return neurite;
}

void NeuronSoma::RemoveDaughter(const SoPointer<NeuriteElement>& daughter) {
  auto it = std::find(std::begin(daughters_), std::end(daughters_), daughter);
  assert(it != std::end(daughters_) &&
         "The element you wanted to remove is not part of daughters_");
  daughters_.erase(it);
}

std::array<double, 3> NeuronSoma::OriginOf(SoUid daughter_uid) const {
  std::array<double, 3> xyz = daughters_coord_.at(daughter_uid);

  double radius = Base::diameter_ * .5;
  xyz = Math::ScalarMult(radius, xyz);

  const auto& pos = Base::position_;

  return {pos[0] + xyz[0] * Base::kXAxis[0] + xyz[1] * Base::kYAxis[0] +
              xyz[2] * Base::kZAxis[0],
          pos[1] + xyz[0] * Base::kXAxis[1] + xyz[1] * Base::kYAxis[1] +
              xyz[2] * Base::kZAxis[1],
          pos[2] + xyz[0] * Base::kXAxis[2] + xyz[1] * Base::kYAxis[2] +
              xyz[2] * Base::kZAxis[2]};
}

void NeuronSoma::UpdateDependentPhysicalVariables() {}

void NeuronSoma::UpdateRelative(const NeuronOrNeurite& old_rel,
                                const NeuronOrNeurite& new_rel) {
  auto old_rel_soptr = old_rel.As<NeuriteElement>()->GetSoPtr<NeuriteElement>();
  auto new_rel_soptr = new_rel.As<NeuriteElement>()->GetSoPtr<NeuriteElement>();
  auto coord = daughters_coord_[old_rel_soptr->GetUid()];
  auto it =
      std::find(std::begin(daughters_), std::end(daughters_), old_rel_soptr);
  assert(it != std::end(daughters_) &&
         "old_element_idx could not be found in daughters_ vector");
  *it = new_rel_soptr;
  daughters_coord_[new_rel_soptr->GetUid()] = coord;
}

const std::vector<SoPointer<NeuriteElement>>& NeuronSoma::GetDaughters() const {
  return daughters_;
}

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm
