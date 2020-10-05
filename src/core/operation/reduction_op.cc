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

#include "core/operation/reduction_op.h"
#include "core/operation/operation_registry.h"

namespace bdm {

BDM_REGISTER_TEMPLATE_OP(ReductionOp, int, "ReductionOpInt", kCpu);
BDM_REGISTER_TEMPLATE_OP(ReductionOp, double, "ReductionOpDouble", kCpu);

}  // namespace bdm
