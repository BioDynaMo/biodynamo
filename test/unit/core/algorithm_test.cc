// -----------------------------------------------------------------------------
//
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

#include "core/algorithm.h"
#include <gtest/gtest.h>
#include <unordered_map>
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

// -----------------------------------------------------------------------------
TEST(ExclusivePrefixSum, Size1) {
  std::vector<uint64_t> v{3};
  decltype(v) expected{0};
  ExclusivePrefixSum(&v, v.size() - 1);
  EXPECT_EQ(expected, v);
}

// -----------------------------------------------------------------------------
TEST(ExclusivePrefixSum, Size9) {
  std::vector<uint64_t> v{1, 2, 3, 4, 5, 6, 7, 0};
  decltype(v) expected{0, 1, 3, 6, 10, 15, 21, 28};
  ExclusivePrefixSum(&v, v.size() - 1);
  EXPECT_EQ(expected, v);
}

// -----------------------------------------------------------------------------
TEST(BinarySearch, Exact) {
  std::vector<uint64_t> v = {0, 1, 2, 3, 4, 5};

  for (uint64_t i = 0; i < v.size(); ++i) {
    EXPECT_EQ(i, BinarySearch(i, v, 0, v.size() - 1));
  }
}

// -----------------------------------------------------------------------------
TEST(BinarySearch, NoExactMatch) {
  std::vector<uint64_t> v = {2, 4, 6};

  EXPECT_EQ(0u, BinarySearch(1u, v, 0, v.size() - 1));
  EXPECT_EQ(0u, BinarySearch(3u, v, 0, v.size() - 1));
  EXPECT_EQ(1u, BinarySearch(5u, v, 0, v.size() - 1));
}

// -----------------------------------------------------------------------------
TEST(BinarySearch, Duplicates) {
  std::vector<uint64_t> v = {1, 1, 1, 1, 2, 2, 2, 3, 3, 4};

  EXPECT_EQ(0u, BinarySearch(0u, v, 0, v.size() - 1));
  EXPECT_EQ(3u, BinarySearch(1u, v, 0, v.size() - 1));
  EXPECT_EQ(6u, BinarySearch(2u, v, 0, v.size() - 1));
  EXPECT_EQ(8u, BinarySearch(3u, v, 0, v.size() - 1));
  EXPECT_EQ(9u, BinarySearch(4u, v, 0, v.size() - 1));
}

// -----------------------------------------------------------------------------
TEST(BinarySearch, DuplicatesLarge) {
  std::vector<uint64_t> v = {
      0,  1,  1,  2,  2,  2,  2,  2,  3,  4,  4,  4,  4,  4,  4,  4,  5,
      5,  5,  5,  6,  7,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  9,  9,
      9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 12,
      12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 16, 16, 16,
      16, 19, 19, 19, 19, 19, 19, 20, 20, 20, 21, 21, 21, 21, 22, 23, 23,
      24, 25, 25, 25, 25, 25, 25, 25, 25, 26, 27, 27, 28, 29, 30, 31, 31,
      31, 31, 31, 33, 34, 34, 34, 34, 34, 35, 35, 35, 36, 36, 36, 36, 36,
      37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 39, 39, 39, 39, 39, 40,
      40, 40, 40, 40, 41, 41, 41, 41, 42};

  // store right-most occurrence in map
  std::unordered_map<uint64_t, uint64_t> map;
  for (uint64_t i = 0; i < v.size(); ++i) {
    map[v[i]] = i;
  }

  // check if right-most occurrence is found
  for (auto& el : map) {
    EXPECT_EQ(el.second, BinarySearch(el.first, v, 0, v.size() - 1));
  }
}

}  // namespace bdm
