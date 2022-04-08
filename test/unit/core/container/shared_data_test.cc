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

#include "unit/core/container/shared_data_test.h"
#include "core/container/shared_data.h"
#include <gtest/gtest.h>
#include <vector>
#include "unit/test_util/io_test.h"

namespace bdm {
namespace shared_data_test {

// Test if resize and size method work correctly.
TEST(SharedDataTest, ReSize) {
  SharedData<int> sdata(10);
  EXPECT_EQ(10u, sdata.size());
  sdata.resize(20);
  EXPECT_EQ(20u, sdata.size());
}

// Test if shared data is occupying full cache lines.
TEST(SharedDataTest, CacheLineAlignment) {
  RunCacheLineAlignmentTest();
}

#ifdef USE_DICT

TEST_F(IOTest, SharedData) {
  RunIOTest();
}

#endif  // USE_DICT

}  // namespace shared_data_test
}  // namespace bdm
