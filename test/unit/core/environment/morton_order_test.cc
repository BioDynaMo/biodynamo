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

#include "core/environment/morton_order.h"
#include <gtest/gtest.h>
#include <morton/morton.h>  // NOLINT

namespace bdm {
namespace morton_order_test_internal {

// -----------------------------------------------------------------------------
void VerifyMortonOrder(const MortonOrder& mo,
                       const MathArray<uint64_t, 3>& num_boxes_axis) {
  auto num_elements = num_boxes_axis[0] * num_boxes_axis[1] * num_boxes_axis[2];

  // all
  std::vector<uint64_t> expected(num_elements);
  uint64_t cnt = 0;
  for (uint64_t z = 0; z < num_boxes_axis[2]; ++z) {
    for (uint64_t y = 0; y < num_boxes_axis[1]; ++y) {
      for (uint64_t x = 0; x < num_boxes_axis[0]; ++x) {
        expected[cnt++] = libmorton::morton3D_64_encode(x, y, z);
      }
    }
  }
  std::sort(expected.begin(), expected.end());
  
  std::vector<uint64_t> actual(num_elements);
  for(uint64_t i = 0; i < num_elements; ++i) {
    actual[i] = mo.GetMortonCode(i);
  }
  EXPECT_EQ(expected, actual);
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, Cube1) {
  MortonOrder mo;
  mo.Update({1, 1, 1});
  VerifyMortonOrder(mo, {1, 1, 1});
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, Cube2) {
  MortonOrder mo;
  mo.Update({2, 2, 2});
  VerifyMortonOrder(mo, {2, 2, 2});
}

// -----------------------------------------------------------------------------
/// Not power of 2
TEST(MortonOrder, Cube3) {
  MortonOrder mo;
  mo.Update({3, 3, 3});
  VerifyMortonOrder(mo, {3, 3, 3});
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, 135) {
  MortonOrder mo;
  mo.Update({1, 3, 5});
  VerifyMortonOrder(mo, {1, 3, 5});
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, 537) {
  MortonOrder mo;
  mo.Update({5, 3, 7});
  VerifyMortonOrder(mo, {5, 3, 7});
}

}  // namespace morton_order_test_internal
}  // namespace bdm
