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
namespace experimental {
namespace neuroscience {

NeuronOrNeurite::~NeuronOrNeurite() {}

SoPointer<NeuronOrNeurite> NeuronOrNeurite::GetNeuronOrNeuriteSoPtr() const {
  if (auto* neuron = As<NeuronSoma>()) {
    return neuron->template GetSoPtr<NeuronOrNeurite>();
  } else if (auto* neurite = As<NeuriteElement>()) {
    return neurite->template GetSoPtr<NeuronOrNeurite>();
  }
  assert(false && "This code should not be reached.");
  return SoPointer<NeuronOrNeurite>();
}

bool NeuronOrNeurite::IsNeuronSoma() const {
  return As<NeuronSoma>() != nullptr;
}

bool NeuronOrNeurite::IsNeuriteElement() const {
  return As<NeuriteElement>() != nullptr;
}

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm
