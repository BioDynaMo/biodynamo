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
void VerifyPositionToOffsetCalc(const MortonOrder& mo,
                                const std::array<uint64_t, 3>& num_boxes_axis) {
  auto num_elements = num_boxes_axis[0] * num_boxes_axis[1] * num_boxes_axis[2];
  std::vector<int> v(num_elements);

  // check first and last
  EXPECT_EQ(0u, mo.GetIndex({0, 0, 0}));
  EXPECT_EQ(num_elements - 1,
            mo.GetIndex({num_boxes_axis[0] - 1, num_boxes_axis[1] - 1,
                         num_boxes_axis[2] - 1}));

  // all
  for (uint64_t z = 0; z < num_boxes_axis[2]; ++z) {
    for (uint64_t y = 0; y < num_boxes_axis[1]; ++y) {
      for (uint64_t x = 0; x < num_boxes_axis[0]; ++x) {
        v[mo.GetIndex({x, y, z})]++;
      }
    }
  }

  for (auto& el : v) {
    EXPECT_EQ(1u, el);
  }
}

// -----------------------------------------------------------------------------
void VerifyBatchedQueries(const MortonOrder& mo,
                          const MathArray<uint64_t, 3>& center) {
  FixedSizeVector<MathArray<uint64_t, 3>, 27> positions;
  for (uint64_t z = -1; z < 2; ++z) {
    for (uint64_t y = -1; y < 2; ++y) {
      for (uint64_t x = -1; x < 2; ++x) {
        MathArray<uint64_t, 3> delta({x, y, z});
        positions.push_back(center + delta);
      }
    }
  }

  auto result = mo.GetIndex(positions);
  for (uint64_t i = 0; i < 27; ++i) {
    EXPECT_EQ(mo.GetIndex({positions[i][0], positions[i][1], positions[i][2]}),
              result[i]);
  }
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, Cube1) {
  MortonOrder mo;
  mo.Update({1, 1, 1});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {};
  EXPECT_EQ(expected, mo.offset_index_);

  EXPECT_EQ(0u, mo.GetIndex({0, 0, 0}));
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, Cube2) {
  MortonOrder mo;
  mo.Update({2, 2, 2});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {};
  EXPECT_EQ(expected, mo.offset_index_);

  // Index calculation
  VerifyPositionToOffsetCalc(mo, {2, 2, 2});
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, Cube1024) {
  MortonOrder mo;
  mo.Update({1024, 1024, 1024});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {};
  EXPECT_EQ(expected, mo.offset_index_);
}

// -----------------------------------------------------------------------------
/// Not power of 2
TEST(MortonOrder, Cube3) {
  MortonOrder mo;
  mo.Update({3, 3, 3});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {
      {9, 0},   {11, 1},  {13, 2},  {15, 3},  {18, 4},  {22, 6}, {25, 8},
      {29, 11}, {36, 14}, {41, 18}, {43, 19}, {50, 24}, {57, 30}};
  EXPECT_EQ(expected, mo.offset_index_);

  // Index calculation
  VerifyPositionToOffsetCalc(mo, {3, 3, 3});
  VerifyBatchedQueries(mo, {1, 1, 1});
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, 135) {
  MortonOrder mo;
  mo.Update({1, 3, 5});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {
      {1, 0},   {3, 1},   {5, 2},     {7, 3},     {17, 12},
      {21, 15}, {33, 26}, {35, 27},   {37, 28},   {39, 29},
      {49, 38}, {53, 41}, {257, 244}, {259, 245}, {273, 258}};
  EXPECT_EQ(expected, mo.offset_index_);

  // Index calculation
  VerifyPositionToOffsetCalc(mo, {1, 3, 5});
}

// -----------------------------------------------------------------------------
TEST(MortonOrder, 537) {
  MortonOrder mo;
  mo.Update({5, 3, 7});

  std::vector<std::pair<uint64_t, uint64_t>> expected = {
      {18, 0},    {22, 2},    {26, 4},    {30, 6},    {50, 8},    {54, 10},
      {58, 12},   {62, 14},   {65, 16},   {67, 17},   {69, 18},   {71, 19},
      {81, 28},   {85, 31},   {97, 42},   {99, 43},   {101, 44},  {103, 45},
      {113, 54},  {117, 57},  {274, 196}, {278, 198}, {282, 200}, {286, 202},
      {292, 204}, {300, 208}, {306, 212}, {314, 218}, {321, 224}, {323, 225},
      {325, 226}, {327, 227}, {337, 236}, {341, 239}, {353, 250}, {355, 251},
      {369, 264}};

  EXPECT_EQ(expected, mo.offset_index_);

  // Index calculation
  VerifyPositionToOffsetCalc(mo, {5, 3, 7});
  VerifyBatchedQueries(mo, {2, 1, 6});
}

}  // namespace morton_order_test_internal
}  // namespace bdm
