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

#ifndef NEUROSCIENCE_NEURON_OR_NEURITE_H_
#define NEUROSCIENCE_NEURON_OR_NEURITE_H_

#include <array>
#include "core/container/math_array.h"
#include "core/sim_object/sim_object.h"
#include "core/sim_object/so_pointer.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

class NeuriteElement;
class NeuronSoma;

/// The mother of a neurite element can either be a neuron or a neurite.
/// This class declares this interface.
class NeuronOrNeurite {
 public:
  virtual ~NeuronOrNeurite();

  virtual SoUid GetUid() const = 0;

  SoPointer<NeuronOrNeurite> GetNeuronOrNeuriteSoPtr() const;

  bool IsNeuronSoma() const;

  bool IsNeuriteElement() const;

  virtual Double3 OriginOf(SoUid daughter_uid) const = 0;

  virtual void RemoveDaughter(const SoPointer<NeuriteElement>& daughter) = 0;

  virtual void UpdateDependentPhysicalVariables() = 0;

  virtual void UpdateRelative(const NeuronOrNeurite& old_rel,
                              const NeuronOrNeurite& new_rel) = 0;
};

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_NEURON_OR_NEURITE_H_
