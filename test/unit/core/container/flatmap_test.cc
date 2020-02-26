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

#include "core/container/flatmap.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(UnorderedFlatmapTest, DefaultCtor) {
  UnorderedFlatmap<int, int> map;
  EXPECT_EQ(0u, map.size());
}

TEST(UnorderedFlatmapTest, SubscriptOperator) {
  UnorderedFlatmap<int, int> map;

  map[2] = 1;
  map[10] = 5;

  EXPECT_EQ(2u, map.size());
  EXPECT_EQ(1, map[2]);
  EXPECT_EQ(5, map[10]);
}

TEST(UnorderedFlatmapTest, Clear) {
  UnorderedFlatmap<int, int> map;

  map[2] = 1;
  map[10] = 5;
  map.clear();

  EXPECT_EQ(0u, map.size());
}

TEST(UnorderedFlatmapTest, At) {
  UnorderedFlatmap<int, int> map;

  map[2] = 1;
  map[10] = 5;

  EXPECT_EQ(2u, map.size());
  EXPECT_EQ(1, map.at(2));
  EXPECT_EQ(5, map.at(10));
}

TEST(UnorderedFlatmapTest, Insert) {
  UnorderedFlatmap<int, int> map;

  map.insert(2, 1);
  map.insert(10, 5);

  EXPECT_EQ(2u, map.size());
  EXPECT_EQ(1, map[2]);
  EXPECT_EQ(5, map[10]);
}

TEST(UnorderedFlatmapTest, Find) {
  UnorderedFlatmap<int, int> map;

  map[2] = 1;
  map[10] = 5;

  auto it = map.find(2);
  EXPECT_TRUE(it != map.end());
  EXPECT_EQ(2, it->first);
  EXPECT_EQ(1, it->second);

  auto it1 = map.find(10);
  EXPECT_TRUE(it1 != map.end());
  EXPECT_EQ(10, it1->first);
  EXPECT_EQ(5, it1->second);

  auto it2 = map.find(42);
  EXPECT_TRUE(it2 == map.end());
}

TEST(UnorderedFlatmapTest, ForEach) {
  UnorderedFlatmap<int, int> map;

  map[2] = 1;
  map[10] = 5;

  int counter = 0;
  for (auto& pair : map) {
    if (counter == 0) {
      EXPECT_EQ(2, pair.first);
      EXPECT_EQ(1, pair.second);
    } else if (counter == 1) {
      EXPECT_EQ(10, pair.first);
      EXPECT_EQ(5, pair.second);
    } else {
      FAIL();
    }
    counter++;
  }
}

TEST(UnorderedFlatmapTest, Reserve) {
  UnorderedFlatmap<int, int> map;

  map[2] = 1;
  map[10] = 5;

  EXPECT_EQ(2u, map.size());

  map.reserve(1);
  EXPECT_EQ(2u, map.size());

  map.reserve(10);
  EXPECT_EQ(2u, map.size());
}

}  // namespace bdm
