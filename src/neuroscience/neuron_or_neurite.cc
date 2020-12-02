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

#include "neuroscience/neuron_or_neurite.h"
#include "neuroscience/neurite_element.h"
#include "neuroscience/neuron_soma.h"

namespace bdm {
namespace neuroscience {

NeuronOrNeurite::~NeuronOrNeurite() {}

AgentPointer<NeuronOrNeurite> NeuronOrNeurite::GetNeuronOrNeuriteAgentPtr()
    const {
  if (auto* neuron = dynamic_cast<const NeuronSoma*>(this)) {
    return neuron->template GetAgentPtr<NeuronOrNeurite>();
  } else if (auto* neurite = dynamic_cast<const NeuriteElement*>(this)) {
    return neurite->template GetAgentPtr<NeuronOrNeurite>();
  }
  assert(false && "This code should not be reached.");
  return AgentPointer<NeuronOrNeurite>();
}

bool NeuronOrNeurite::IsNeuronSoma() const {
  return dynamic_cast<const NeuronSoma*>(this) != nullptr;
}

bool NeuronOrNeurite::IsNeuriteElement() const {
  return dynamic_cast<const NeuriteElement*>(this) != nullptr;
}

}  // namespace neuroscience
}  // namespace bdm
