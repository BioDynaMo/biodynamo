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

#include "core/container/shared_data.h"
#include "core/real.h"
#include <gtest/gtest.h>
#include <vector>

namespace bdm {

// Test if resize and size method work correctly.
TEST(SharedDataTest, ReSize) {
  SharedData<int> sdata(10);
  EXPECT_EQ(10u, sdata.size());
  sdata.resize(20);
  EXPECT_EQ(20u, sdata.size());
}

// Test if shared data is occupying full cache lines.
TEST(SharedDataTest, CacheLineAlignment) {
  // Test standard data tyes int, float, real
  // Test alignment of int
  EXPECT_EQ(
      std::alignment_of<typename SharedData<int>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test alignment of float
  EXPECT_EQ(
      std::alignment_of<typename SharedData<float>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test alignment of real
  EXPECT_EQ(
      std::alignment_of<typename SharedData<real>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test size of vector components int
  EXPECT_EQ(sizeof(typename SharedData<int>::Data::value_type),
            BDM_CACHE_LINE_SIZE);
  // Test size of vector components float
  EXPECT_EQ(sizeof(typename SharedData<float>::Data::value_type),
            BDM_CACHE_LINE_SIZE);
  // Test size of vector components real
  EXPECT_EQ(sizeof(typename SharedData<real>::Data::value_type),
            BDM_CACHE_LINE_SIZE);

  // Test a chache line fully filled with reals.
  // Test alignment of real[max_real], e.g. max cache line capacity
  EXPECT_EQ(
      std::alignment_of<
          typename SharedData<real[BDM_CACHE_LINE_SIZE /
                                     sizeof(real)]>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test size of vector components real[max_real], e.g. max cache line
  // capacity
  EXPECT_EQ(
      sizeof(typename SharedData<
             real[BDM_CACHE_LINE_SIZE / sizeof(real)]>::Data::value_type),
      BDM_CACHE_LINE_SIZE);

  // Test some custom data structures
  // Test alignment of data that fills 1 cache line
  EXPECT_EQ(std::alignment_of<typename SharedData<
                char[BDM_CACHE_LINE_SIZE - 1]>::Data::value_type>::value,
            BDM_CACHE_LINE_SIZE);
  // Test alignment of data that fills 2 cache lines
  EXPECT_EQ(std::alignment_of<typename SharedData<
                char[BDM_CACHE_LINE_SIZE + 1]>::Data::value_type>::value,
            BDM_CACHE_LINE_SIZE);
  // Test alignment of data that fills 3 cache lines
  EXPECT_EQ(std::alignment_of<typename SharedData<
                char[2 * BDM_CACHE_LINE_SIZE + 1]>::Data::value_type>::value,
            BDM_CACHE_LINE_SIZE);
  // Test size of data that fills 1 cache line
  EXPECT_EQ(
      sizeof(
          typename SharedData<char[BDM_CACHE_LINE_SIZE - 1]>::Data::value_type),
      BDM_CACHE_LINE_SIZE);
  // Test size of data that fills 2 cache lines
  EXPECT_EQ(
      sizeof(
          typename SharedData<char[BDM_CACHE_LINE_SIZE + 1]>::Data::value_type),
      2 * BDM_CACHE_LINE_SIZE);
  // Test size of data that fills 3 cache lines
  EXPECT_EQ(sizeof(typename SharedData<
                   char[2 * BDM_CACHE_LINE_SIZE + 1]>::Data::value_type),
            3 * BDM_CACHE_LINE_SIZE);
}

}  // namespace bdm
