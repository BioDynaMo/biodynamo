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

#include "core/agent/agent_handle.h"
#include <gtest/gtest.h>

namespace bdm {

// -----------------------------------------------------------------------------
TEST(AgentHandle, DefaultCtor) {
  AgentHandle ah;
  EXPECT_EQ(std::numeric_limits<AgentHandle::PrimaryIndex_t>::max(),
            ah.GetPrimaryIndex());
  EXPECT_EQ(std::numeric_limits<AgentHandle::SecondaryIndex_t>::max(),
            ah.GetSecondaryIndex());
  EXPECT_FALSE(ah.IsInAura());
}

// -----------------------------------------------------------------------------
TEST(AgentHandle, Ctor1) {
  AgentHandle ah(12);
  EXPECT_EQ(0u, ah.GetPrimaryIndex());
  EXPECT_EQ(12u, ah.GetSecondaryIndex());
  EXPECT_FALSE(ah.IsInAura());
}

// -----------------------------------------------------------------------------
TEST(AgentHandle, Ctor2) {
  AgentHandle ah(12, 34);
  EXPECT_EQ(12u, ah.GetPrimaryIndex());
  EXPECT_EQ(34u, ah.GetSecondaryIndex());
  EXPECT_FALSE(ah.IsInAura());
}

// -----------------------------------------------------------------------------
TEST(AgentHandle, Ctor3) {
  AgentHandle ah(true, 12, 34);
  EXPECT_EQ(12u, ah.GetPrimaryIndex());
  EXPECT_EQ(34u, ah.GetSecondaryIndex());
  EXPECT_TRUE(ah.IsInAura());
}

// -----------------------------------------------------------------------------
TEST(AgentHandle, Equals) {
  EXPECT_EQ(AgentHandle(), AgentHandle());
  EXPECT_EQ(AgentHandle(12, 34), AgentHandle(12, 34));
  EXPECT_EQ(AgentHandle(true, 12, 34), AgentHandle(true, 12, 34));
  EXPECT_FALSE(AgentHandle(false, 12, 34) == AgentHandle(true, 12, 34));
  EXPECT_FALSE(AgentHandle(true, 13, 34) == AgentHandle(true, 12, 34));
  EXPECT_FALSE(AgentHandle(true, 12, 35) == AgentHandle(true, 12, 34));
}

// -----------------------------------------------------------------------------
TEST(AgentHandle, LessThan) {
  // equals
  EXPECT_FALSE(AgentHandle() < AgentHandle());
  EXPECT_FALSE(AgentHandle(12, 34) < AgentHandle(12, 34));
  EXPECT_FALSE(AgentHandle(true, 12, 34) < AgentHandle(true, 12, 34));

  // greater
  EXPECT_FALSE(AgentHandle(true, 12, 34) < AgentHandle(false, 12, 34));
  EXPECT_FALSE(AgentHandle(true, 13, 34) < AgentHandle(true, 12, 34));
  EXPECT_FALSE(AgentHandle(true, 12, 35) < AgentHandle(true, 12, 34));

  // less than
  EXPECT_TRUE(AgentHandle(false, 13, 35) < AgentHandle(true, 12, 34));
  EXPECT_TRUE(AgentHandle(true, 11, 35) < AgentHandle(true, 12, 34));
  EXPECT_TRUE(AgentHandle(true, 12, 34) < AgentHandle(true, 12, 35));
}

}  // namespace bdm
