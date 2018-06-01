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

#include "unit/biology_module_op_test.h"
#include "gtest/gtest.h"

namespace bdm {
namespace biology_module_op_test_internal {

TEST(BiologyModuleOpTest, ComputeAos) { RunTestAos(); }

TEST(BiologyModuleOpTest, ComputeSoa) { RunTestSoa(); }

}  // namespace biology_module_op_test_internal
}  // namespace bdm
