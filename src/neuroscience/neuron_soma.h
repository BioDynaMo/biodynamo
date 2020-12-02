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

#include "core/agent/cell.h"
#include "core/container/math_array.h"
#include "neuroscience/neuron_or_neurite.h"

namespace bdm {
namespace neuroscience {

class NeuriteElement;

class NeuronSoma : public Cell, public NeuronOrNeurite {
  BDM_AGENT_HEADER(NeuronSoma, Cell, 1);

 public:
  NeuronSoma();
  virtual ~NeuronSoma();

  explicit NeuronSoma(const Double3& position);

  NeuronSoma(const NeuronSoma& other)
      : Base(other),
        daughters_(other.daughters_),
        daughters_coord_(other.daughters_coord_) {}

  /// \brief This method is used to initialise the values of daughter
  /// 2 for a cell division event.
  ///
  /// Please note that this implementation does not allow division of neuron
  /// somas with already attached neurite elements.
  ///
  /// \see CellDivisionEvent
  void Initialize(const NewAgentEvent& event) override;

  /// \brief This method is used to update attributes after a cell division.
  /// or new neurite branching event.
  ///
  /// Performs the transition mother to daughter 1
  /// \see Event, CellDivisionEvent
  void Update(const NewAgentEvent& event) override;

  const AgentUid& GetUid() const override { return Base::GetUid(); }

  Spinlock* GetLock() override { return Base::GetLock(); }

  void CriticalRegion(std::vector<Spinlock*>* locks) override;

  // ***************************************************************************
  //      METHODS FOR NEURON TREE STRUCTURE *
  // ***************************************************************************

  /// \brief Extend a new neurite from this soma.
  ///
  /// Uses default diameter for new neurite
  /// \see NewNeuriteExtensionEvent
  NeuriteElement* ExtendNewNeurite(const Double3& direction,
                                   NeuriteElement* prototype = nullptr);

  /// \brief Extend a new neurite from this soma.
  ///
  /// \see NewNeuriteExtensionEvent
  NeuriteElement* ExtendNewNeurite(double diameter, double phi, double theta,
                                   NeuriteElement* prototype = nullptr);

  void RemoveDaughter(const AgentPointer<NeuriteElement>& daughter) override;

  /// Returns the absolute coordinates of the location where the daughter is
  /// attached.
  /// @param daughter_element_idx element_idx of the daughter
  /// @return the coord
  Double3 OriginOf(const AgentUid& daughter_uid) const override;

  void UpdateDependentPhysicalVariables() override;

  void UpdateRelative(const NeuronOrNeurite& old_rel,
                      const NeuronOrNeurite& new_rel) override;

  const std::vector<AgentPointer<NeuriteElement>>& GetDaughters() const;

 protected:
  std::vector<AgentPointer<NeuriteElement>> daughters_;

  /// Daughter attachment points in local coordinates
  /// Key: neurite segment uid
  /// Value: position
  std::unordered_map<AgentUid, Double3> daughters_coord_;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_SOMA_H_
