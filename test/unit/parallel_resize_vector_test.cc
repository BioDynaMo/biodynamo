// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#include "parallel_resize_vector.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(ParallelResizeVectorTest, Basics) {
  ParallelResizeVector<unsigned> v = {0, 1, 2, 3};
  ASSERT_EQ(4u, v.size());

  // push_back
  v.push_back(4);
  ASSERT_EQ(5u, v.size());

  // subscript operator
  for (unsigned i = 0; i < v.size(); i++) {
    EXPECT_EQ(i, v[i]);
  }

  // iterator
  unsigned cnt = 0;
  for (auto& el : v) {
    EXPECT_EQ(cnt++, el);
  }
  EXPECT_EQ(5u, cnt);

  // clear
  v.clear();
  EXPECT_EQ(0u, v.size());

  // resize
  v.resize(10);
  ASSERT_EQ(10u, v.size());
  for (unsigned i = 0; i < v.size(); i++) {
    EXPECT_EQ(0u, v[i]);
  }
}

TEST(ParallelResizeVector, Reserve) {
  ParallelResizeVector<int> v;
  EXPECT_EQ(0u, v.size());
  EXPECT_EQ(0u, v.capacity());

  v.reserve(123);
  EXPECT_EQ(0u, v.size());
  EXPECT_EQ(123u, v.capacity());
}

}  // namespace bdm
