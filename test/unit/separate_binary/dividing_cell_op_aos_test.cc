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

#include "core/param/compile_time_param.h"
#include "core/simulation_implementation.h"
#include "unit/core/operation/dividing_cell_op_test.h"

namespace bdm {

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();
  using SimulationBackend = Scalar;
};

namespace dividing_cell_op_test_internal {

TEST(DividingCellOpTest, ComputeAos) { RunTest<Cell>(); }

}  // namespace dividing_cell_op_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
