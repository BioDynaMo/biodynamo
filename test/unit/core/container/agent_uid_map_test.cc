// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "core/container/agent_uid_map.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(AgentUidMapTest, DefaultCtor) {
  AgentUidMap<int> map;
  EXPECT_EQ(map.size(), 0u);
}

TEST(AgentUidMapTest, Ctor) {
  AgentUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for (int i = 0; i < 10; ++i) {
    EXPECT_FALSE(map.Contains(AgentUid(i)));
  }
}

TEST(AgentUidMapTest, CopyCtor) {
  AgentUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for (int i = 0; i < 10; ++i) {
    map.Insert(AgentUid(i), i);
  }

  AgentUidMap<int> copy(map);
  EXPECT_EQ(copy.size(), 10u);

  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(copy.Contains(AgentUid(i)));
    EXPECT_EQ(copy[AgentUid(i)], i);
  }
}

TEST(AgentUidMapTest, AddElements) {
  AgentUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for (int i = 0; i < 10; ++i) {
    map.Insert(AgentUid(i), i);
  }

  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(map.Contains(AgentUid(i)));
    EXPECT_EQ(map[AgentUid(i)], i);
  }
}

TEST(AgentUidMapTest, Remove) {
  AgentUidMap<int> map(5);

  for (unsigned i = 0; i < map.size(); ++i) {
    map.Insert(AgentUid(i), i);
  }

  map.Remove(AgentUid(0));
  map.Remove(AgentUid(2));
  map.Remove(AgentUid(4));

  EXPECT_FALSE(map.Contains(AgentUid(0)));
  EXPECT_FALSE(map.Contains(AgentUid(2)));
  EXPECT_FALSE(map.Contains(AgentUid(4)));

  EXPECT_TRUE(map.Contains(AgentUid(1)));
  EXPECT_TRUE(map.Contains(AgentUid(3)));
}

TEST(AgentUidMapTest, ParallelClear) {
  AgentUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for (int i = 0; i < 10; ++i) {
    map.Insert(AgentUid(i), i);
  }

  map.ParallelClear();

  for (int i = 0; i < 10; ++i) {
    EXPECT_FALSE(map.Contains(AgentUid(i)));
  }
}

TEST(AgentUidMapTest, resize) {
  AgentUidMap<int> map(10);
  EXPECT_EQ(map.size(), 10u);

  for (int i = 0; i < 10; ++i) {
    map.Insert(AgentUid(i), i);
  }

  map.resize(20);
  EXPECT_EQ(map.size(), 20u);

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(map[AgentUid(i)], i);
  }

  for (int i = 10; i < 20; ++i) {
    EXPECT_FALSE(map.Contains(AgentUid(i)));
  }
}

}  // namespace bdm
