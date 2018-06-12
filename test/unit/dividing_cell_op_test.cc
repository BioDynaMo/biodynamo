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

#include "unit/dividing_cell_op_test.h"
#include "unit/default_ctparam.h"
#include "bdm_imp.h"

namespace bdm {
namespace dividing_cell_op_test_internal {

// ComputeAos test is in different binary, because it requires different compile
// time parameter (separate_binary/dividing_cell_op_aos_test.cc)

TEST(DividingCellOpTest, ComputeSoa) { RunTest<Cell>(); }

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm
