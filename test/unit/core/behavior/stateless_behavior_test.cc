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

#include "core/behavior/stateless_behavior.h"
#include <gtest/gtest.h>
#include "core/agent/cell_division_event.h"
#include "unit/test_util/io_test.h"
#include "unit/test_util/test_agent.h"
#include "unit/test_util/test_util.h"

namespace bdm {

// -----------------------------------------------------------------------------
TEST(StatelessBehavior, DefaultCtor) {
  // Tests that the behavior doesn't crash if no function pointer is given.
  StatelessBehavior b;
  b.Run(nullptr);
}

// -----------------------------------------------------------------------------
TEST(StatelessBehavior, MainCtor) {
  Simulation sim(TEST_NAME);

  StatelessBehavior b(
      [](Agent* a) { bdm_static_cast<TestAgent*>(a)->SetData(123); });
  TestAgent a;
  b.Run(&a);
  EXPECT_EQ(123, a.GetData());
}

// -----------------------------------------------------------------------------
TEST(StatelessBehavior, CopyCtor) {
  uint64_t one = 1;
  Simulation sim(TEST_NAME);

  StatelessBehavior b(
      [](Agent* a) { bdm_static_cast<TestAgent*>(a)->SetData(123); });

  b.CopyToNewIf({one << 2});
  b.RemoveFromExistingIf({one << 5});

  StatelessBehavior bcopy(b);

  TestAgent a;
  bcopy.Run(&a);
  EXPECT_EQ(123, a.GetData());

  // check if base class got copied correctly
  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = one << i;
    if (i != 2) {
      EXPECT_FALSE(bcopy.WillBeCopied(e));
    } else {
      EXPECT_TRUE(bcopy.WillBeCopied(e));
    }
    if (i != 5) {
      EXPECT_FALSE(bcopy.WillBeRemoved(e));
    } else {
      EXPECT_TRUE(bcopy.WillBeRemoved(e));
    }
  }
}

// -----------------------------------------------------------------------------
TEST(StatelessBehavior, Event) {
  uint64_t one = 1;
  Simulation sim(TEST_NAME);

  StatelessBehavior b(
      [](Agent* a) { bdm_static_cast<TestAgent*>(a)->SetData(123); });

  b.CopyToNewIf({one << 2});
  b.RemoveFromExistingIf({one << 5});

  // simulate event
  CellDivisionEvent event(1, 2, 3);
  event.existing_behavior = &b;
  auto* bnew = b.New();
  bnew->Initialize(event);

  TestAgent a;
  bnew->Run(&a);
  EXPECT_EQ(123, a.GetData());

  // check if base class got copied correctly
  for (uint64_t i = 0; i < 64; i++) {
    NewAgentEventUid e = one << i;
    if (i != 2) {
      EXPECT_FALSE(bnew->WillBeCopied(e));
    } else {
      EXPECT_TRUE(bnew->WillBeCopied(e));
    }
    if (i != 5) {
      EXPECT_FALSE(bnew->WillBeRemoved(e));
    } else {
      EXPECT_TRUE(bnew->WillBeRemoved(e));
    }
  }
}

#ifdef USE_DICT
// -----------------------------------------------------------------------------
TEST_F(IOTest, StatelessBehavior) {
  Simulation sim(TEST_NAME);

  StatelessBehavior b(
      [](Agent* a) { bdm_static_cast<TestAgent*>(a)->SetData(123); });

  StatelessBehavior* restored;
  BackupAndRestore(b, &restored);

  TestAgent a;
  restored->Run(&a);
  EXPECT_EQ(123, a.GetData());
}

#endif  // USE_DICT

}  // namespace bdm
