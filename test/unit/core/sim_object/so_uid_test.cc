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
#include "unit/test_util/io_test.h"

namespace bdm {

TEST(SoUidTest, DefaultCtor) {
  SoUid uid;
  EXPECT_EQ(uid.GetIndex(), std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(uid.GetReused(), std::numeric_limits<uint32_t>::max());
}

TEST(SoUidTest, CtorOneParam) {
  SoUid uid(123);
  EXPECT_EQ(uid.GetIndex(), 123u);
  EXPECT_EQ(uid.GetReused(), 0u);
}

TEST(SoUidTest, CtorTwoParam) {
  SoUid uid(123, 456);
  EXPECT_EQ(uid.GetIndex(), 123u);
  EXPECT_EQ(uid.GetReused(), 456u);
}

TEST(SoUidTest, EqualsOperator) {
  SoUid uid(123, 456);
  EXPECT_TRUE(uid == SoUid(123, 456));
  EXPECT_FALSE(uid == SoUid(456, 123));
  EXPECT_FALSE(uid == SoUid(789, 987));

  EXPECT_FALSE(uid != SoUid(123, 456));
  EXPECT_TRUE(uid != SoUid(456, 123));
  EXPECT_TRUE(uid != SoUid(789, 987));
}

TEST(SoUidTest, LessThanOperator) {
  SoUid uid(123, 456);
  EXPECT_FALSE(uid < uid);
  EXPECT_FALSE(uid < SoUid(123, 455));
  EXPECT_TRUE(uid < SoUid(123, 457));

  EXPECT_FALSE(uid < SoUid(122, 456));
  EXPECT_TRUE(uid < SoUid(124, 456));
}

TEST(SoUidTest, PlosOperator) {
  SoUid uid(123, 456);

  // integer
  {
    auto new_uid = uid + 7;
    EXPECT_EQ(new_uid, SoUid(130, 456));
  }

  // unsigned integer
  {
    auto new_uid = uid + static_cast<uint64_t>(7u);
    EXPECT_EQ(new_uid, SoUid(130, 456));
  }
}

TEST(SoUidTest, MinusOperator) {
  SoUid uid(123, 456);

  // integer
  {
    auto new_uid = uid - 7;
    EXPECT_EQ(new_uid, SoUid(116, 456));
  }

  // unsigned integer
  {
    auto new_uid = uid - static_cast<uint64_t>(7u);
    EXPECT_EQ(new_uid, SoUid(116, 456));
  }
}

TEST(SoUidTest, uint64_tOperator) {
  SoUid uid(123, 0);
  uint64_t idx = uid;
  EXPECT_EQ(idx, 123u);
}

TEST(SoUidTest, uint64_tOperator2) {
  SoUid uid(123, 2);
  uint64_t idx = uid;
  EXPECT_EQ(idx, 8589934715u); //(2 << 32) | 123u);
}

#ifdef USE_DICT
TEST_F(IOTest, SoUid) {
  SoUid test{123u, 456u};
  SoUid* restored = nullptr;

  BackupAndRestore(test, &restored);

  EXPECT_EQ(restored->GetIndex(), 123u);
  EXPECT_EQ(restored->GetReused(), 456u);

  delete restored;
}
#endif  // USE_DICT

}  // namespace bdm
