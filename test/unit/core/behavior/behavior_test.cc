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
struct TestBehavior : public BaseBehavior {
  TestBehavior() : BaseBehavior(0, 0) {}
  explicit TestBehavior(EventId copy_event, EventId remove_event = 0)
      : BaseBehavior(copy_event, remove_event) {}

  TestBehavior(std::initializer_list<EventId> copy_events,
                    std::initializer_list<EventId> remove_events = {})
      : BaseBehavior(copy_events, remove_events) {}

  TestBehavior(const Event& event, BaseBehavior* other,
                    uint64_t new_oid = 0)
      : BaseBehavior(event, other, new_oid) {}

  virtual ~TestBehavior() {}

  void Run(Agent* agent) override {}

  BaseBehavior* GetInstance(const Event& event, BaseBehavior* other,
                                 uint64_t new_oid = 0) const override {
    return new TestBehavior(event, other, new_oid);
  }
  BaseBehavior* GetCopy() const override {
    return new TestBehavior(*this);
  };
};

TEST(BaseBehaviorTest, CopyNever) {
  TestBehavior bbm;

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_FALSE(bbm.Copy(e));
  }
}

TEST(BaseBehaviorTest, CopyAlways) {
  TestBehavior bbm(gAllEventIds);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_TRUE(bbm.Copy(e));
  }
}

TEST(BaseBehaviorTest, CopyOnSingleEvent) {
  uint64_t one = 1;
  TestBehavior bbm(one << 5);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5) {
      EXPECT_FALSE(bbm.Copy(e));
    } else {
      EXPECT_TRUE(bbm.Copy(e));
    }
  }
}

TEST(BaseBehaviorTest, CopyOnEventList) {
  uint64_t one = 1;
  TestBehavior bbm({one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(bbm.Copy(e));
    } else {
      EXPECT_TRUE(bbm.Copy(e));
    }
  }
}

TEST(BaseBehaviorTest, RemoveNever) {
  TestBehavior bbm;
  EventId any = 1;
  TestBehavior bbm1(any, gNullEventId);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_FALSE(bbm.Remove(e));
    EXPECT_FALSE(bbm1.Remove(e));
  }
}

TEST(BaseBehaviorTest, RemoveAlways) {
  EventId any = 1;
  TestBehavior bbm(any, gAllEventIds);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_TRUE(bbm.Remove(e));
  }
}

TEST(BaseBehaviorTest, RemoveOnSingleEvent) {
  uint64_t one = 1;
  EventId any = 1;
  TestBehavior bbm(any, one << 5);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5) {
      EXPECT_FALSE(bbm.Remove(e));
    } else {
      EXPECT_TRUE(bbm.Remove(e));
    }
  }
}

TEST(BaseBehaviorTest, RemoveOnEventList) {
  uint64_t one = 1;
  EventId any = 1;
  TestBehavior bbm({any}, {one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(bbm.Remove(e));
    } else {
      EXPECT_TRUE(bbm.Remove(e));
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
