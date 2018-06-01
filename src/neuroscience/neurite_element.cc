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

#include "neuroscience/neurite_element.h"

namespace bdm {
namespace experimental {
namespace neuroscience {

const BmEvent gNeuriteElongation =
    UniqueBmEventFactory::Get()->NewUniqueBmEvent();
const BmEvent gNeuriteBranching =
    UniqueBmEventFactory::Get()->NewUniqueBmEvent();
const BmEvent gNeuriteBifurcation =
    UniqueBmEventFactory::Get()->NewUniqueBmEvent();
const BmEvent gNeuriteSideCylinderExtension =
    UniqueBmEventFactory::Get()->NewUniqueBmEvent();

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm
