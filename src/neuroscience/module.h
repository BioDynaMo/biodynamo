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

#ifndef NEUROSCIENCE_MODULE_H_
#define NEUROSCIENCE_MODULE_H_

namespace bdm {
namespace experimental {
namespace neuroscience {

/// Initializes the neuroscience module.
/// NB: Must be called before `bdm::Simulation` is created!
void InitModule();

}  // namespace neuroscience
}  // namespace experimental
}  // namespace bdm

#endif  // NEUROSCIENCE_MODULE_H_
