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

#include "unit/core/operation/mechanical_forces_op_test.h"
#include "gtest/gtest.h"

namespace bdm {
namespace mechanical_forces_op_test_internal {

TEST(DisplacementOpTest, ComputeUniformGrid) { RunTest("uniform_grid"); }
TEST(DisplacementOpTest, ComputeKDTree) { RunTest("kd_tree"); }
TEST(DisplacementOpTest, ComputeOctree) { RunTest("octree"); }

TEST(DisplacementOpTest, ComputeNewUniformGrid) { RunTest2("uniform_grid"); }
TEST(DisplacementOpTest, ComputeNewKDTree) { RunTest2("kd_tree"); }
TEST(DisplacementOpTest, ComputeNewOctree) { RunTest2("octree"); }

}  // namespace mechanical_forces_op_test_internal
}  // namespace bdm
