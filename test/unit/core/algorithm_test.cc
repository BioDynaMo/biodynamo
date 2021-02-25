// -----------------------------------------------------------------------------
//
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

#include "core/algorithm.h"
#include <gtest/gtest.h>
#include <vector>

namespace bdm {

// -----------------------------------------------------------------------------
void CalcExpected(std::vector<uint64_t>* v) {
  for (uint64_t i = 1; i < v->size(); ++i) {
    (*v)[i] += (*v)[i - 1];
  }
}

// -----------------------------------------------------------------------------
TEST(InPlaceParallelPrefixSum, Empty) {
  std::vector<uint64_t> v{};
  decltype(v) expected = v;
  InPlaceParallelPrefixSum(v, v.size());
  CalcExpected(&expected);
  EXPECT_EQ(expected, v);
}

// -----------------------------------------------------------------------------
TEST(InPlaceParallelPrefixSum, Size1) {
  std::vector<uint64_t> v{3};
  decltype(v) expected = v;
  InPlaceParallelPrefixSum(v, v.size());
  CalcExpected(&expected);
  EXPECT_EQ(expected, v);
}

// -----------------------------------------------------------------------------
TEST(InPlaceParallelPrefixSum, MultipleOf2) {
  std::vector<uint64_t> v{0, 1, 2, 3, 4, 5, 6, 7};
  decltype(v) expected = v;
  InPlaceParallelPrefixSum(v, v.size());
  CalcExpected(&expected);
  EXPECT_EQ(expected, v);
}

// -----------------------------------------------------------------------------
TEST(InPlaceParallelPrefixSum, Size9) {
  std::vector<uint64_t> v{0, 1, 2, 3, 4, 5, 6, 7, 8};
  decltype(v) expected = v;
  InPlaceParallelPrefixSum(v, v.size());
  CalcExpected(&expected);
  EXPECT_EQ(expected, v);
}

}  // namespace bdm
