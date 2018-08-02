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

#ifndef DEMO_DISTRIBUTED_BACKEND_H_
#define DEMO_DISTRIBUTED_BACKEND_H_

#include "biodynamo.h"

namespace bdm {

/// Defines the storage backend used by BioDynaMo. This file should be included
/// at the very top.
template<typename Backend>
struct CompileTimeParam : public DefaultCompileTimeParam<Backend> {
  // use predefined biology module GrowDivide
  using BiologyModules = Variant<GrowDivide>;
  // use default Backend and AtomicTypes
  using SimulationBackend = Scalar;
};

} // namespace bdm

#endif  // DEMO_DISTRIBUTED_BACKEND_H_
