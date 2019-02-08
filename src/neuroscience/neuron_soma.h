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

#ifndef NEUROSCIENCE_NEURON_SOMA_H_
#define NEUROSCIENCE_NEURON_SOMA_H_

// #include <algorithm>
// #include <unordered_map>
// #include <vector>
// #include "core/resource_manager.h"
// #include "core/sim_object/cell.h"
// #include "neuroscience/event/new_neurite_extension_event.h"
// #include "neuroscience/param.h"
// #include "neuroscience/neuron_or_neurite.h"
//
// namespace bdm {
// namespace experimental {
// namespace neuroscience {
//
// class NeuronSoma : public Cell, public NeuronOrNeurite {
//   BDM_SIM_OBJECT_HEADER(NeuronSoma, Cell, 1, daughters_, daughters_coord_);
//
//  public:
//   NeuronSoma() {}
//   virtual ~NeuronSoma() {}
//
//   explicit NeuronSoma(const std::array<double, 3>& position) : Base(position) {}
//
//   /// \brief This EventConstructor is used to initialise the values of daughter
//   /// 2 for a cell division event.
//   ///
//   /// Please note that  this implementation does not allow division of neuron
//   /// somas with already attached neurite elements.
//   ///
//   /// \see CellDivisionEvent
//   void EventConstructor(const Event& event, SimObject* mother_so, uint64_t new_oid = 0) override {
//
//     Base::EventConstructor(event, mother, new_oid);
//
//     const CellDivisionEvent* cdevent = dynamic_cast<const CellDivisionEvent*>(&event);
//     NeuronSoma* mother = dynamic_cast<Cell*>(mother_so);
//     if(cdevent && mother) {
//       if (mother->daughters_.size() != 0) {
//         Fatal("NeuronSoma",
//               "Dividing a neuron soma with attached neurites is not supported "
//               "in the default implementation! If you want to change this "
//               "behavior derive from this class and overwrite this constructor.");
//       }
//     }
//   }
//
//   /// \brief EventHandler to modify the data members of this cell
//   /// after a cell division.
//   ///
//   /// Performs the transition mother to daughter 1
//   /// \param event contains parameters for cell division
//   /// \param daughter_2 pointer to new cell (=daughter 2)
//   /// \see Event, CellDivisionEvent
//   void EventHandler(const Event& event, SimObject *other1, SimObject* other2 = nullptr) override {
//     Base::EventHandler(event, other1, other2);
//   }
//
//   /// \brief EventHandler to modify the data members of this soma
//   /// after a new neurite extension event.
//   ///
//   /// Performs the transition mother to daughter 1
//   /// \param event contains parameters for new neurite extension
//   /// \param neurite pointer to new neurite
//   /// \see NewNeuriteExtensionEvent
//   // template <typename TNeurite>
//   // void EventHandler(const NewNeuriteExtensionEvent& event, TNeurite* neurite) {
//   void EventHandler(const Event& event, SimObject *other1, SimObject* other2 = nullptr) override {
//     // forward to SimObject::EventHandler
//     Base::Base::EventHandler(event, neurite);
//
//     const NewNeuriteExtensionEvent* ne_event = dynamic_cast<const NewNeuriteExtensionEvent*>(&event);
//     NeuriteElement* neurite = dynamic_cast<NeuriteElement*>(other1);
//     if(ne_event && mother) {
//       double theta = ne_event->theta_;
//       double phi = ne_event->phi_;
//       double x_coord = std::sin(theta) * std::cos(phi);
//       double y_coord = std::sin(theta) * std::sin(phi);
//       double z_coord = std::cos(theta);
//
//       daughters_.push_back(neurite->GetSoPtr());
//       daughters_coord_[neurite->GetUid()] = {x_coord, y_coord, z_coord};
//     }
//   }
//
//   // ***************************************************************************
//   //      METHODS FOR NEURON TREE STRUCTURE *
//   // ***************************************************************************
//
//   /// \brief Extend a new neurite from this soma.
//   ///
//   /// Uses default diameter for new neurite
//   /// \see NewNeuriteExtensionEvent
//   NeuriteElement* ExtendNewNeurite(const std::array<double, 3>& direction) {
//     auto dir = Math::Add(direction, Base::position_);
//     auto angles = Base::TransformCoordinatesGlobalToPolar(dir);
//     auto* param = Simulation::GetActive()->GetParam()->GetModuleParam<Param>();
//     return ExtendNewNeurite(param->neurite_default_diameter_, angles[2],
//                             angles[1]);
//   }
//
//   /// \brief Extend a new neurite from this soma.
//   ///
//   /// \see NewNeuriteExtensionEvent
//   NeuriteElement* ExtendNewNeurite(double diameter, double phi, double theta) {
//     auto* ctxt = Simulation::GetActive()->GetExecutionContext();
//     NewNeuriteExtensionEvent event(diameter, phi, theta);
//     NeuriteElement* neurite = new NeuriteElement();
//     neurite->EventConstructor(event, this);
//     ctxt->push_back(neurite);
//     EventHandler(event, neurite);
//     return neurite;
//   }
//
//   void RemoveDaughter(const SoPointer<NeuriteElement>& daughter) override {
//     auto it = std::find(std::begin(daughters_),
//                         std::end(daughters_), daughter);
//     assert(it != std::end(daughters_) &&
//            "The element you wanted to remove is not part of daughters_");
//     daughters_.erase(it);
//   }
//
//   /// Returns the absolute coordinates of the location where the daughter is
//   /// attached.
//   /// @param daughter_element_idx element_idx of the daughter
//   /// @return the coord
//   std::array<double, 3> OriginOf(SoUid daughter_uid) const override {
//     std::array<double, 3> xyz = daughters_coord_[daughter_uid];
//
//     double radius = Base::diameter_ * .5;
//     xyz = Math::ScalarMult(radius, xyz);
//
//     const auto& pos = Base::position_;
//
//     return {pos[0] + xyz[0] * Base::kXAxis[0] + xyz[1] * Base::kYAxis[0] +
//                 xyz[2] * Base::kZAxis[0],
//             pos[1] + xyz[0] * Base::kXAxis[1] + xyz[1] * Base::kYAxis[1] +
//                 xyz[2] * Base::kZAxis[1],
//             pos[2] + xyz[0] * Base::kXAxis[2] + xyz[1] * Base::kYAxis[2] +
//                 xyz[2] * Base::kZAxis[2]};
//   }
//
//   void UpdateRelative(const SoPointer<NeuriteElement>& old_rel,
//                       const SoPointer<NeuriteElement>& new_rel) override {
//     auto coord = daughters_coord_[old_rel->GetUid()];
//     auto it = std::find(std::begin(daughters_),
//                         std::end(daughters_), old_rel);
//     assert(it != std::end(daughters_) &&
//            "old_element_idx could not be found in daughters_ vector");
//     *it = new_rel;
//     daughters_coord_[new_rel->GetUid()] = coord;
//   }
//
//   const std::vector<SoPointer<NeuriteElement>>& GetDaughters() const {
//     return daughters_;
//   }
//
//  protected:
//   std::vector<SoPointer<NeuriteElement>> daughters_;
//
//   /// Daughter attachment points in local coordinates
//   /// Key: neurite segment uid
//   /// Value: position
//   std::unordered_map<SoUid, std::array<double, 3>> daughters_coord_;
//
// };
//
// }  // namespace neuroscience
// }  // namespace experimental
// }  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_SOMA_H_
