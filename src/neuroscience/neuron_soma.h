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

#include <unordered_map>
#include <vector>
#include "core/sim_object/cell.h"
#include "neuroscience/neuron_or_neurite.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

class NeuronSoma : public Cell, public NeuronNeurite {
  BDM_SIM_OBJECT_HEADER(NeuronSoma, Cell, 1, daughters_, daughters_coord_);

 public:
  NeuronSoma();
  virtual ~NeuronSoma();

  explicit NeuronSoma(const std::array<double, 3>& position);

  /// \brief This EventConstructor is used to initialise the values of daughter
  /// 2 for a cell division event.
  ///
  /// Please note that  this implementation does not allow division of neuron
  /// somas with already attached neurite elements.
  ///
  /// \see CellDivisionEvent
  void EventConstructor(const Event& event, SimObject* mother_so, uint64_t new_oid = 0) override;

  /// \brief EventHandler to modify the data members of this cell
  /// after a cell division, or new neurite branching event
  ///
  /// Performs the transition mother to daughter 1
  /// \param event contains parameters for cell division
  /// \param daughter_2 pointer to new cell (=daughter 2)
  /// \see Event, CellDivisionEvent
  void EventHandler(const Event& event, SimObject *other1, SimObject* other2 = nullptr) override;

  // ***************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE *
  // ***************************************************************************

  /// \brief Extend a new neurite from this soma.
  ///
  /// Uses default diameter for new neurite
  /// \see NewNeuriteExtensionEvent
  NeuriteElement* ExtendNewNeurite(const std::array<double, 3>& direction);

  /// \brief Extend a new neurite from this soma.
  ///
  /// \see NewNeuriteExtensionEvent
  NeuriteElement* ExtendNewNeurite(double diameter, double phi, double theta);

  void RemoveDaughter(const SoPointer<NeuriteElement>& daughter) override;

  /// Returns the absolute coordinates of the location where the daughter is
  /// attached.
  /// @param daughter_element_idx element_idx of the daughter
  /// @return the coord
  std::array<double, 3> OriginOf(SoUid daughter_uid) const override;

  void UpdateDependentPhysicalVariables() override;

  void UpdateRelative(const SoPointer<NeuriteElement>& old_rel,
                      const SoPointer<NeuriteElement>& new_rel) override;

  const std::vector<SoPointer<NeuriteElement>>& GetDaughters() const;

 protected:
  std::vector<SoPointer<NeuriteElement>> daughters_;

  /// Daughter attachment points in local coordinates
  /// Key: neurite segment uid
  /// Value: position
  std::unordered_map<SoUid, std::array<double, 3>> daughters_coord_;

};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_SOMA_H_
