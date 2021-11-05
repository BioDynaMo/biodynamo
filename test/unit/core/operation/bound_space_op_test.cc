// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/operation/bound_space_op.h"
#include <gtest/gtest.h>
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {

// -----------------------------------------------------------------------------
TEST(BoundSpaceOp, Torus) {
  Simulation sim(TEST_NAME);

  TestAgent a;
  a.SetPosition({-15, 0, 50});
  ApplyBoundingBox(&a, Param::BoundSpaceMode::kTorus, -10, 30);
  EXPECT_ARR_NEAR({25, 0, 10}, a.GetPosition());
}

// -----------------------------------------------------------------------------
TEST(BoundSpaceOp, TorusDistanceLargerThanSim) {
  Simulation sim(TEST_NAME);

  TestAgent a;
  a.SetPosition({-200, 0, 150});
  ApplyBoundingBox(&a, Param::BoundSpaceMode::kTorus, -10, 30);
  EXPECT_ARR_NEAR({0, 0, -10}, a.GetPosition());
}

}  // namespace bdm
