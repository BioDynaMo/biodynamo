// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include "core/container/math_array.h"
#include "core/operation/reduction_op.h"
#include "core/operation/operation_registry.h"

namespace bdm {

BDM_REGISTER_TEMPLATE_OP(ReductionOp, int, "ReductionOpInt", kCpu);
BDM_REGISTER_TEMPLATE_OP(ReductionOp, double, "ReductionOpDouble", kCpu);
BDM_REGISTER_TEMPLATE_OP(ReductionOp, Double3, "ReductionOpDouble3", kCpu);
BDM_REGISTER_TEMPLATE_OP(ReductionOp, Double4, "ReductionOpDouble4", kCpu);

}  // namespace bdm
