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

#include "core/behavior/behavior.h"
#include <gtest/gtest.h>

namespace bdm {

/// Helper class to test run visitor
struct TestBehavior : public Behavior {
  TestBehavior() : Behavior(0, 0) {}
  explicit TestBehavior(EventId copy_event, EventId remove_event = 0)
      : Behavior(copy_event, remove_event) {}

  TestBehavior(std::initializer_list<EventId> copy_events,
                    std::initializer_list<EventId> remove_events = {})
      : Behavior(copy_events, remove_events) {}

  TestBehavior(const Event& event, Behavior* other,
                    uint64_t new_oid = 0)
      : Behavior(event, other, new_oid) {}

  virtual ~TestBehavior() {}

  void Run(Agent* agent) override {}

  Behavior* GetInstance(const Event& event, Behavior* other,
                                 uint64_t new_oid = 0) const override {
    return new TestBehavior(event, other, new_oid);
  }
  Behavior* GetCopy() const override {
    return new TestBehavior(*this);
  };
};

TEST(BehaviorTest, CopyNever) {
  TestBehavior b;

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_FALSE(b.Copy(e));
  }
}

TEST(BehaviorTest, CopyAlways) {
  TestBehavior b(gAllEventIds);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_TRUE(b.Copy(e));
  }
}

TEST(BehaviorTest, CopyOnSingleEvent) {
  uint64_t one = 1;
  TestBehavior b(one << 5);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5) {
      EXPECT_FALSE(b.Copy(e));
    } else {
      EXPECT_TRUE(b.Copy(e));
    }
  }
}

TEST(BehaviorTest, CopyOnEventList) {
  uint64_t one = 1;
  TestBehavior b({one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(b.Copy(e));
    } else {
      EXPECT_TRUE(b.Copy(e));
    }
  }
}

TEST(BehaviorTest, RemoveNever) {
  TestBehavior b;
  EventId any = 1;
  TestBehavior bbm1(any, gNullEventId);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_FALSE(b.Remove(e));
    EXPECT_FALSE(bbm1.Remove(e));
  }
}

TEST(BehaviorTest, RemoveAlways) {
  EventId any = 1;
  TestBehavior b(any, gAllEventIds);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_TRUE(b.Remove(e));
  }
}

TEST(BehaviorTest, RemoveOnSingleEvent) {
  uint64_t one = 1;
  EventId any = 1;
  TestBehavior b(any, one << 5);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5) {
      EXPECT_FALSE(b.Remove(e));
    } else {
      EXPECT_TRUE(b.Remove(e));
    }
  }
}

TEST(BehaviorTest, RemoveOnEventList) {
  uint64_t one = 1;
  EventId any = 1;
  TestBehavior b({any}, {one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(b.Remove(e));
    } else {
      EXPECT_TRUE(b.Remove(e));
    }
  }
}

TEST(UniqueEventIdFactoryTest, All) {
  auto uef = UniqueEventIdFactory::Get();

  auto event_id_1 = uef->NewUniqueEventId();
  auto event_id_2 = uef->NewUniqueEventId();

  EXPECT_EQ(event_id_1, event_id_2 >> 1);
}

}  // namespace bdm
