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

#include <algorithm>
#include <unordered_map>
#include <vector>
#include "cell.h"
#include "neuroscience/event/new_neurite_extension_event.h"
#include "neuroscience/param.h"
#include "resource_manager.h"
#include "simulation_object_util.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

BDM_SIM_OBJECT(NeuronSoma, Cell) {
  BDM_SIM_OBJECT_HEADER(NeuronSoma, Cell, 1, daughters_, daughters_coord_);

 public:
  using NeuriteElement = typename TCompileTimeParam::NeuriteElement;
  using NeuriteElementSoPtr = ToSoPtr<NeuriteElement>;

  NeuronSomaExt() {}

  explicit NeuronSomaExt(const std::array<double, 3>& position)
      : Base(position) {}

  /// \brief This constructor is used to create daughter 2 for a cell division
  /// event.
  ///
  /// Please note that  this implementation does not allow division of neuron
  /// somas with already attached neurite elements.
  ///
  /// \see CellDivisionEvent
  template <typename TMother>
  NeuronSomaExt(const CellDivisionEvent& event, TMother* mother,
                uint64_t new_oid = 0)
      : Base(event, mother) {
    if (mother->daughters_[mother->kIdx].size() != 0) {
      Fatal("NeuronSoma",
            "Dividing a neuron soma with attached neurites is not supported "
            "in the default implementation! If you want to change this "
            "behavior derive from this class and overwrite this constructor.");
    }
  }

  /// \brief EventHandler to modify the data members of this neuron soma
  /// after a cell division event.
  ///
  /// Performs the transition mother to daughter 1
  /// \param event contains parameters for cell division
  /// \param daughter_2 pointer to new cell ( = daughter 2)
  /// \see CellDivisionEvent
  template <typename TDaughter>
  void EventHandler(const CellDivisionEvent& event, TDaughter* daughter_2) {
    Base::EventHandler(event, daughter_2);
  }

  // ***************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE *
  // ***************************************************************************

  /// \brief Extend a new neurite from this soma.
  ///
  /// Uses default diameter for new neurite
  /// \see NewNeuriteExtensionEvent
  NeuriteElementSoPtr ExtendNewNeurite(const std::array<double, 3>& direction) {
    auto dir = Math::Add(direction, Base::position_[kIdx]);
    auto angles = Base::TransformCoordinatesGlobalToPolar(dir);
    auto* param = Simulation_t::GetActive()->GetParam();
    return ExtendNewNeurite(param->neurite_default_diameter_, angles[2],
                            angles[1]);
  }

  /// \brief Extend a new neurite from this soma.
  ///
  /// \see NewNeuriteExtensionEvent
  NeuriteElementSoPtr ExtendNewNeurite(double diameter, double phi,
                                       double theta) {
    auto* rm = Simulation_t::GetActive()->GetResourceManager();
    NewNeuriteExtensionEvent event = {diameter, phi, theta};
    auto&& neurite = rm->template New<NeuriteElement>(event, ThisMD());
    ThisMD()->EventHandler(event, &neurite);
    return neurite.GetSoPtr();
  }

  void RemoveDaughter(const ToSoPtr<NeuriteElement> daughter) {
    auto it = std::find(std::begin(daughters_[kIdx]),
                        std::end(daughters_[kIdx]), daughter);
    assert(it != std::end(daughters_[kIdx]) &&
           "The element you wanted to remove is not part of daughters_[kIdx]");
    daughters_[kIdx].erase(it);
  }

  /// Returns the absolute coordinates of the location where the daughter is
  /// attached.
  /// @param daughter_element_idx element_idx of the daughter
  /// @return the coord
  std::array<double, 3> OriginOf(uint32_t daughter_element_idx) const {
    std::array<double, 3> xyz = daughters_coord_[kIdx][daughter_element_idx];

    double radius = Base::diameter_[kIdx] * .5;
    xyz = Math::ScalarMult(radius, xyz);

    const auto& pos = Base::position_[kIdx];

    return {pos[0] + xyz[0] * Base::kXAxis[0] + xyz[1] * Base::kYAxis[0] +
                xyz[2] * Base::kZAxis[0],
            pos[1] + xyz[0] * Base::kXAxis[1] + xyz[1] * Base::kYAxis[1] +
                xyz[2] * Base::kZAxis[1],
            pos[2] + xyz[0] * Base::kXAxis[2] + xyz[1] * Base::kYAxis[2] +
                xyz[2] * Base::kZAxis[2]};
  }

  void UpdateRelative(const ToSoPtr<NeuriteElement>& old_rel,
                      const ToSoPtr<NeuriteElement>& new_rel) {
    auto coord = daughters_coord_[kIdx][old_rel->GetElementIdx()];
    auto it = std::find(std::begin(daughters_[kIdx]),
                        std::end(daughters_[kIdx]), old_rel);
    assert(it != std::end(daughters_[kIdx]) &&
           "old_element_idx could not be found in daughters_ vector");
    *it = new_rel;
    daughters_coord_[kIdx][new_rel->GetElementIdx()] = coord;
  }

  const std::vector<ToSoPtr<NeuriteElement>>& GetDaughters() const {
    return daughters_[kIdx];
  }

 protected:
  vec<std::vector<ToSoPtr<NeuriteElement>>> daughters_ = {{}};

  /// Daughter attachment points in local coordinates
  /// Key: element index of neurite segement
  /// Value: position
  vec<std::unordered_map<uint32_t, std::array<double, 3>>> daughters_coord_ = {
      {}};

  /// \brief EventHandler to modify the data members of this soma
  /// after a new neurite extension event.
  ///
  /// Performs the transition mother to daughter 1
  /// \param event contains parameters for new neurite extension
  /// \param neurite pointer to new neurite
  /// \see NewNeuriteExtensionEvent
  template <typename TNeurite>
  void EventHandler(const NewNeuriteExtensionEvent& event, TNeurite* neurite) {
    // forward to SimulationObject::EventHandler
    Base::Base::EventHandler(event, neurite);

    double theta = event.theta_;
    double phi = event.phi_;
    double x_coord = std::sin(theta) * std::cos(phi);
    double y_coord = std::sin(theta) * std::sin(phi);
    double z_coord = std::cos(theta);

    daughters_[kIdx].push_back(neurite->GetSoPtr());
    daughters_coord_[kIdx][neurite->GetElementIdx()] = {x_coord, y_coord,
                                                        z_coord};
  }
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_SOMA_H_
