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

#include "unit/test_util/test_util.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(TestUtilTest, abs_errorTypeTrait) {
  EXPECT_FLOAT_EQ(abs_error<float>::value, 1e-4);
  EXPECT_DOUBLE_EQ(abs_error<double>::value, 1e-9);
  // abs_error<int>::value // must not compile -> todo compile error test suite
}

}  // namespace bdm
