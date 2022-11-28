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

#ifndef NEUROSCIENCE_NEURON_OR_NEURITE_H_
#define NEUROSCIENCE_NEURON_OR_NEURITE_H_

#include <array>
#include "core/agent/agent.h"
#include "core/agent/agent_pointer.h"
#include "core/container/math_array.h"

namespace bdm {

class Spinlock;

namespace neuroscience {

class NeuriteElement;
class NeuronSoma;

enum StructureIdentifierSWC {
  kUndefined,
  kSoma,
  kAxon,
  kBasalDendrite,
  kApicalDendrite
};

/// The mother of a neurite element can either be a neuron or a neurite.
/// This class declares this interface.
class NeuronOrNeurite {
 public:
  virtual ~NeuronOrNeurite();

  virtual const AgentUid& GetUid() const = 0;
  virtual Spinlock* GetLock() = 0;

  AgentPointer<NeuronOrNeurite> GetNeuronOrNeuriteAgentPtr() const;

  /// \brief Returns the SWC classification of the object.
  ///
  /// \see StructureIdentifierSWC. Returns StructureIdentifierSWC:kSoma for the
  /// Soma and StructureIdentifierSWC::kApicalDendrite for all other objects.
  /// May be modified in child classes.
  virtual StructureIdentifierSWC GetIdentifierSWC() const;

  bool IsNeuronSoma() const;

  bool IsNeuriteElement() const;

  virtual Real3 OriginOf(const AgentUid& daughter_uid) const = 0;

  virtual void RemoveDaughter(const AgentPointer<NeuriteElement>& daughter) = 0;

  virtual void UpdateDependentPhysicalVariables() = 0;

  virtual void UpdateRelative(const NeuronOrNeurite& old_rel,
                              const NeuronOrNeurite& new_rel) = 0;
};

}  // namespace neuroscience
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_OR_NEURITE_H_
