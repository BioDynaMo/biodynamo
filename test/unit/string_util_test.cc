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

#include "string_util.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(StringUtilTest, Concat) {
  EXPECT_EQ("foobarbaz", Concat("foo", "bar", "baz"));
  EXPECT_EQ("foo bar baz", Concat("foo", " bar ", "baz"));
  EXPECT_EQ("foo1 bar baz", Concat("foo", 1, " bar ", "baz"));
  EXPECT_EQ("foo 3.14 bar baz", Concat("foo ", 3.14, " bar ", "baz"));
}

}  // namespace bdm
