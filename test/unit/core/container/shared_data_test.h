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

#ifndef UNIT_CORE_CONTAINER_SHARED_DATA_H_
#define UNIT_CORE_CONTAINER_SHARED_DATA_H_

#include "core/container/shared_data.h"
#include <gtest/gtest.h>
#include "unit/test_util/io_test.h"

namespace bdm {
namespace shared_data_test {

inline void RunCacheLineAlignmentTest() {
  // Test standard data types int, float, double
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

  // Test a cache line fully filled with doubles.
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

#ifdef USE_DICT

inline void RunIOTest() {
  SharedData<int> sd;
  sd.resize(10);

  for(uint64_t i = 0; i < sd.size(); ++i) {
    sd[i] = 3 * i;
  } 

  SharedData<int>* restored;
  BackupAndRestore(sd, &restored);

  ASSERT_EQ(10u, restored->size());
  for(uint64_t i = 0; i < restored->size(); ++i) {
    EXPECT_EQ(static_cast<int>(3 * i), (*restored)[i]);
  } 
}

#endif  // USE_DICT

}  // namespace shared_data_test
}  // namespace bdm

#endif  // UNIT_CORE_CONTAINER_SHARED_DATA_H_

