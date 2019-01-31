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

#include "unit/core/biology_module/biology_module_test.h"

namespace bdm {

TEST(BiologyModuleUtilTest, RunVisitor) { RunRunVisitor(); }

TEST(BaseBiologyModuleTest, CopyNever) {
  BaseBiologyModule bbm;

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_FALSE(bbm.Copy(e));
  }
}

TEST(BaseBiologyModuleTest, CopyAlways) {
  BaseBiologyModule bbm(gAllEventIds);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_TRUE(bbm.Copy(e));
  }
}

TEST(BaseBiologyModuleTest, CopyOnSingleEvent) {
  uint64_t one = 1;
  BaseBiologyModule bbm(one << 5);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5) {
      EXPECT_FALSE(bbm.Copy(e));
    } else {
      EXPECT_TRUE(bbm.Copy(e));
    }
  }
}

TEST(BaseBiologyModuleTest, CopyOnEventList) {
  uint64_t one = 1;
  BaseBiologyModule bbm({one << 5, one << 19, one << 49});

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5 && i != 19 && i != 49) {
      EXPECT_FALSE(bbm.Copy(e));
    } else {
      EXPECT_TRUE(bbm.Copy(e));
    }
  }
}

TEST(BaseBiologyModuleTest, RemoveNever) {
  BaseBiologyModule bbm;
  EventId any = 1;
  BaseBiologyModule bbm1(any, gNullEventId);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_FALSE(bbm.Remove(e));
    EXPECT_FALSE(bbm1.Remove(e));
  }
}

TEST(BaseBiologyModuleTest, RemoveAlways) {
  EventId any = 1;
  BaseBiologyModule bbm(any, gAllEventIds);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = 1 << i;
    EXPECT_TRUE(bbm.Remove(e));
  }
}

TEST(BaseBiologyModuleTest, RemoveOnSingleEvent) {
  uint64_t one = 1;
  EventId any = 1;
  BaseBiologyModule bbm(any, one << 5);

  for (uint64_t i = 0; i < 64; i++) {
    EventId e = one << i;
    if (i != 5) {
      EXPECT_FALSE(bbm.Remove(e));
    } else {
      EXPECT_TRUE(bbm.Remove(e));
    }
  }
}

TEST(BaseBiologyModuleTest, RemoveOnEventList) {
  uint64_t one = 1;
  EventId any = 1;
  BaseBiologyModule bbm({any}, {one << 5, one << 19, one << 49});

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
