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

#include "neuroscience/neuron_soma.h"

#include <algorithm>
#include <ctime>
#include <locale>

#include "core/resource_manager.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/new_agent_event/new_neurite_extension_event.h"
#include "neuroscience/param.h"

namespace bdm {
namespace neuroscience {

NeuronSoma::NeuronSoma() = default;

NeuronSoma::~NeuronSoma() = default;

NeuronSoma::NeuronSoma(const Real3& position) : Base(position) {}

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

    real_t theta = ne_event.theta;
    real_t phi = ne_event.phi;
    real_t x_coord = std::sin(theta) * std::cos(phi);
    real_t y_coord = std::sin(theta) * std::sin(phi);
    real_t z_coord = std::cos(theta);

    daughters_.push_back(neurite->GetAgentPtr<NeuriteElement>());
    daughters_coord_[neurite->GetUid()] = {x_coord, y_coord, z_coord};
  }

  // do nothing for CellDivisionEvent or others
}

void NeuronSoma::CriticalRegion(std::vector<AgentPointer<>>* aptrs) {
  aptrs->reserve(daughters_.size() + 1);
  aptrs->push_back(Agent::GetAgentPtr<>());
  for (auto& daughter : daughters_) {
    aptrs->push_back(daughter);
  }
}

NeuriteElement* NeuronSoma::ExtendNewNeurite(const Real3& direction,
                                             NeuriteElement* prototype) {
  auto dir = direction + GetPosition();
  auto angles = Base::TransformCoordinatesGlobalToPolar(dir);
  auto* param = Simulation::GetActive()->GetParam()->Get<Param>();
  return ExtendNewNeurite(param->neurite_default_diameter, angles[2], angles[1],
                          prototype);
}

NeuriteElement* NeuronSoma::ExtendNewNeurite(real_t diameter, real_t phi,
                                             real_t theta,
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

Real3 NeuronSoma::OriginOf(const AgentUid& daughter_uid) const {
  Real3 xyz = daughters_coord_.at(daughter_uid);

  real_t radius = GetDiameter() * .5;
  xyz = xyz * radius;

  Real3 axis_0 = {Base::kXAxis[0], Base::kYAxis[0], Base::kZAxis[0]};
  Real3 axis_1 = {Base::kXAxis[1], Base::kYAxis[1], Base::kZAxis[1]};
  Real3 axis_2 = {Base::kXAxis[2], Base::kYAxis[2], Base::kZAxis[2]};

  Real3 result = {xyz * axis_0, xyz * axis_1, xyz * axis_2};
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

// Helper function for PrintSWC to print the elements of a vector separated by
// a space.
inline void PrintVector(std::ostream& out, const Real3& pos) {
  out << " " << pos[0] << " " << pos[1] << " " << pos[2] << " ";
}

void NeuronSoma::PrintSWC(std::ostream& out) const {
  // 1. Write a header to the file such that users do not confuse it with real
  //    data.
  out << "# This SWC file was created with BioDynaMo and is (very) likely "
         "to\n# be the result of a simulation. Be careful not to confuse it\n"
         "# with real data. \n\n";
  // Example for localtime_r: https://tinyurl.com/localtimer
  struct tm newtime;
  time_t ltime;
  ltime = time(&ltime);
  localtime_r(&ltime, &newtime);
  char creation_time[100];
  std::strftime(creation_time, sizeof(creation_time), "%A, %F, %T", &newtime);
  out << "# Created at: " << creation_time << "\n\n";

  // 2. Write the line for the soma at the beginning of the file
  int element_id{1};
  int previous_element_id{-1};
  out << element_id++ << " " << GetIdentifierSWC();
  PrintVector(out, GetPosition());
  out << GetDiameter() / 2 << " " << previous_element_id << "\n";

  // 3. Iterate over all neuron branches that are attached to the soma
  for (auto daughter : daughters_) {
    // New branches need to be connected to soma
    bool is_new_brach{true};
    // Pointers to currently considered and next element
    AgentPointer<NeuriteElement> current_element, next_element;
    // Vector to track the bifurcation elements (right daughters that are
    // neglected when going down the left path, see below)
    std::vector<AgentPointer<NeuriteElement>> bifurcation_points;
    // Track the labels that we need to connect the bifurcation points to the
    // appropriate elements
    std::vector<int> bifurcation_points_lables;
    // Used to reconnect to previous bifurcation
    bool connect_to_bifurcation{false};

    // The logic of the while loop is the following. The tree of dendrites /
    // axons is made up of subsequent elements. Each element has a left daughter
    // but only bifurcations points also have a right daughter. If a branch
    // ends, the element has neither a left nor a right daughter. Thus, for each
    // branch attached to the soma, we walk down the tree all the way down
    // following the left path. On the way, we remember the bifurcation points.
    // Once we've reached the end of a branch, we jump back to the last
    // bifurcation point that we have visited and start to follow the left path
    // again. With this method we can resolve the entire tree structure under
    // the assumption, that each element has 0, 1, or 2 follow up elements and
    // that different branches do not merge again.
    current_element = daughter;
    while (true) {
      // Loop 1. Write down the SWC line corresponding to the current element.
      //         The file format per line is as follows:
      //         <element id> <type id> <x> <y> <z> <radius> <prev element id>
      out << element_id << " " << current_element->GetIdentifierSWC();
      PrintVector(out, current_element->GetPosition());
      out << current_element->GetDiameter() / 2 << " ";
      // This block establishes the connection via <prev element id>
      if (is_new_brach) {
        // New branches need to be attached to the soma
        previous_element_id = 1;
        is_new_brach = false;
      } else {
        if (!connect_to_bifurcation) {
          // By default we connect to the previously considered element
          previous_element_id = element_id - 1;
        } else {
          // At some points we need to reconnect the branches to the previous
          // bifurcation point.
          previous_element_id = bifurcation_points_lables.back();
          bifurcation_points_lables.pop_back();
          connect_to_bifurcation = false;
        }
      }
      out << previous_element_id << "\n";

      // Loop 2. Walk down one side of the tree, take left option first, and
      // remember bifurcation points.
      next_element = current_element->GetDaughterLeft();
      // Track the bifurcations
      if (current_element->GetDaughterRight() != nullptr) {
        bifurcation_points.push_back(current_element->GetDaughterRight());
        bifurcation_points_lables.push_back(element_id);
      }
      element_id++;  // Increase ID by 1
      if (next_element == nullptr && bifurcation_points.empty()) {
        // If we have resolved all bifurcations and don't have any next element,
        // we exit the loop
        break;
      } else if (next_element == nullptr) {
        // Jump to previous bifurcation
        next_element = bifurcation_points.back();
        bifurcation_points.pop_back();
        connect_to_bifurcation = true;
      }
      std::swap(current_element, next_element);  // Swap Pointers
    }
  }
}

}  // namespace neuroscience
}  // namespace bdm
