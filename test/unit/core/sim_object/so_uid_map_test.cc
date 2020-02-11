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

#include <gtest/gtest.h>
#include "core/sim_object/so_uid.h"

namespace bdm {

TEST(SoUidMapTest, DefaultCtor) {
  SoUidMap<int> map;
  EXPECT_EQ(map.size(), 0u);
}

TEST(SoUidMapTest, Ctor) {
  SoUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for(int i = 0; i < 10; ++i) {
    EXPECT_FALSE(map.Contains(SoUid(i)));
  }
}

TEST(SoUidMapTest, CopyCtor) {
  SoUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for(int i = 0; i < 10; ++i) {
    map.Insert(SoUid(i), i);
  }

  SoUidMap<int> copy(map);
  EXPECT_EQ(copy.size(), 10u);

  for(int i = 0; i < 10; ++i) {
    EXPECT_TRUE(copy.Contains(SoUid(i)));
    EXPECT_EQ(copy[SoUid(i)], i);
  }
}

TEST(SoUidMapTest, AddElements) {
  SoUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for(int i = 0; i < 10; ++i) {
    map.Insert(SoUid(i), i);
  }

  for(int i = 0; i < 10; ++i) {
    EXPECT_TRUE(map.Contains(SoUid(i)));
    EXPECT_EQ(map[SoUid(i)], i);
  }
}

TEST(SoUidMapTest, Remove) {
  SoUidMap<int> map(5);

  for(unsigned i = 0; i < map.size(); ++i) {
    map.Insert(SoUid(i), i);
  }

  map.Remove(SoUid(0));
  map.Remove(SoUid(2));
  map.Remove(SoUid(4));

  EXPECT_FALSE(map.Contains(SoUid(0)));
  EXPECT_FALSE(map.Contains(SoUid(2)));
  EXPECT_FALSE(map.Contains(SoUid(4)));

  EXPECT_TRUE(map.Contains(SoUid(1)));
  EXPECT_TRUE(map.Contains(SoUid(3)));
}

TEST(SoUidMapTest, ParallelClear) {
  SoUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for(int i = 0; i < 10; ++i) {
    map.Insert(SoUid(i), i);
  }

  map.ParallelClear();

  for(int i = 0; i < 10; ++i) {
    EXPECT_FALSE(map.Contains(SoUid(i)));
  }
}

TEST(SoUidMapTest, resize) {
  SoUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for(int i = 0; i < 10; ++i) {
    map.Insert(SoUid(i), i);
  }

  map.resize(20);
  EXPECT_EQ(map.size(), 20u);


  for(int i = 0; i < 10; ++i) {
    EXPECT_EQ(map[SoUid(i)], i);
  }

  for(int i = 10; i < 20; ++i) {
    EXPECT_FALSE(map.Contains(SoUid(i)));
  }
}

}  // namespace bdm
