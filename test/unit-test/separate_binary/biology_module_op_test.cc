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

#include "unit/separate_binary/biology_module_op_test.h"
#include "gtest/gtest.h"
#include "simulation_implementation.h"

namespace bdm {
namespace biology_module_op_test_internal {

TEST(BiologyModuleOpTest, ComputeAos) { RunTestAos(); }

TEST(BiologyModuleOpTest, ComputeSoa) { RunTestSoa(); }

}  // namespace biology_module_op_test_internal
}  // namespace bdm

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
