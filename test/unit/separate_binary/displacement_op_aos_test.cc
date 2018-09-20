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

#include "compile_time_param.h"
#include "gtest/gtest.h"
#include "simulation_implementation.h"
#include "unit/displacement_op_test.h"

namespace bdm {

BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();

  using SimulationBackend = Scalar;
};

namespace displacement_op_test_internal {

TEST(DisplacementOpTest, ComputeAos) { RunTest(); }

}  // namespace displacement_op_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
