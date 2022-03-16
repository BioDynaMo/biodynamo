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

#include "core/behavior/behavior.h"
#include <gtest/gtest.h>

namespace bdm {

/// Helper class to test run visitor
struct TestBehavior : public Behavior {
  TestBehavior() = default;

  virtual ~TestBehavior() = default;

  void Run(Agent* agent) override {}

  Behavior* New() const override { return new TestBehavior(); }
  Behavior* NewCopy() const override { return new TestBehavior(*this); }
};

TEST(BehaviorTest, CopyNever) {
  TestBehavior b;
  TestBehavior b1;
  b1.AlwaysCopyToNew();
  b1.NeverCopyToNew();

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = 1ull << i;
    EXPECT_FALSE(b.WillBeCopied(e));
  }
}

TEST(BehaviorTest, CopyAlways) {
  TestBehavior b;
  b.AlwaysCopyToNew();

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = 1ull << i;
    EXPECT_TRUE(b.WillBeCopied(e));
  }
}

TEST(BehaviorTest, CopyOnSingleEvent) {
  uint64_t one = 1;
  TestBehavior b;
  b.CopyToNewIf({one << 5});

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = one << i;
    if (i != 5) {
      EXPECT_FALSE(b.WillBeCopied(e));
    } else {
      EXPECT_TRUE(b.WillBeCopied(e));
    }
  }
}

TEST(BehaviorTest, CopyOnEventList) {
  uint64_t one = 1;
  TestBehavior b;
  b.CopyToNewIf({one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(b.WillBeCopied(e));
    } else {
      EXPECT_TRUE(b.WillBeCopied(e));
    }
  }
}

TEST(BehaviorTest, RemoveNever) {
  TestBehavior b;
  TestBehavior b1;
  b1.NeverRemoveFromExisting();

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = 1ull << i;
    EXPECT_FALSE(b.WillBeRemoved(e));
    EXPECT_FALSE(b1.WillBeRemoved(e));
  }
}

TEST(BehaviorTest, RemoveAlways) {
  TestBehavior b;
  b.AlwaysRemoveFromExisting();

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = 1ull << i;
    EXPECT_TRUE(b.WillBeRemoved(e));
  }
}

TEST(BehaviorTest, RemoveOnSingleEvent) {
  uint64_t one = 1;
  TestBehavior b;
  b.RemoveFromExistingIf({one << 5});

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = one << i;
    if (i != 5) {
      EXPECT_FALSE(b.WillBeRemoved(e));
    } else {
      EXPECT_TRUE(b.WillBeRemoved(e));
    }
  }
}

TEST(BehaviorTest, RemoveOnEventList) {
  uint64_t one = 1;
  TestBehavior b;
  b.RemoveFromExistingIf({one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(b.WillBeRemoved(e));
    } else {
      EXPECT_TRUE(b.WillBeRemoved(e));
    }
  }
}

TEST(NewAgentEventUidGeneratorTest, All) {
  auto uef = NewAgentEventUidGenerator::GetInstance();

  auto event_id_1 = uef->GenerateUid();
  auto event_id_2 = uef->GenerateUid();

  EXPECT_EQ(event_id_1, event_id_2 >> 1);
}

}  // namespace bdm
