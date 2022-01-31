// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#ifndef CORE_OPERATION_MECHANICAL_FORCES_OP_OPENCL_H_
#define CORE_OPERATION_MECHANICAL_FORCES_OP_OPENCL_H_

#include "core/agent/cell.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects
struct MechanicalForcesOpOpenCL : StandaloneOperationImpl {
  BDM_OP_HEADER(MechanicalForcesOpOpenCL);

  void IsNonSphericalObjectPresent(const Agent* agent, bool* answer);

  void operator()() override;
};

}  // namespace bdm

#endif  // CORE_OPERATION_MECHANICAL_FORCES_OP_OPENCL_H_
