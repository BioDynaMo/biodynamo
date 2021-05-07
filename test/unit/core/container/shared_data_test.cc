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

#include "core/container/shared_data.h"
#include <gtest/gtest.h>
#include <vector>

namespace bdm {

// Test if resize and size method work correctly.
TEST(SharedDataTest, ReSize) {
  SharedData<int> sdata(10);
  EXPECT_EQ(sdata.size(), 10);
  sdata.resize(20);
  EXPECT_EQ(sdata.size(), 20);
}

// Test if shared data is occupying full cache lines.
TEST(SharedDataTest, CacheLineAlignment) {
  // Test standard data tyes int, float, double
  // Test alignment of int
  EXPECT_EQ(
      std::alignment_of<typename SharedData<int>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test alignment of float
  EXPECT_EQ(
      std::alignment_of<typename SharedData<float>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test alignment of double
  EXPECT_EQ(
      std::alignment_of<typename SharedData<double>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test size of vector components int
  EXPECT_EQ(sizeof(typename SharedData<int>::Data::value_type),
            BDM_CACHE_LINE_SIZE);
  // Test size of vector components float
  EXPECT_EQ(sizeof(typename SharedData<float>::Data::value_type),
            BDM_CACHE_LINE_SIZE);
  // Test size of vector components double
  EXPECT_EQ(sizeof(typename SharedData<double>::Data::value_type),
            BDM_CACHE_LINE_SIZE);

  // Test a chache line fully filled with doubles.
  // Test alignment of double[max_double], e.g. max cache line capacity
  EXPECT_EQ(
      std::alignment_of<
          typename SharedData<double[BDM_CACHE_LINE_SIZE /
                                     sizeof(double)]>::Data::value_type>::value,
      BDM_CACHE_LINE_SIZE);
  // Test size of vector components double[max_double], e.g. max cache line
  // capacity
  EXPECT_EQ(
      sizeof(typename SharedData<
             double[BDM_CACHE_LINE_SIZE / sizeof(double)]>::Data::value_type),
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
