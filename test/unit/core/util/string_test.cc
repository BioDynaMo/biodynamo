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

#include "core/util/string.h"
#include <gtest/gtest.h>

namespace bdm {

TEST(StringUtilTest, Concat) {
  EXPECT_EQ("foobarbaz", Concat("foo", "bar", "baz"));
  EXPECT_EQ("foo bar baz", Concat("foo", " bar ", "baz"));
  EXPECT_EQ("foo1 bar baz", Concat("foo", 1, " bar ", "baz"));
  EXPECT_EQ("foo 3.14 bar baz", Concat("foo ", 3.14, " bar ", "baz"));
}

TEST(StringUtilTest, EndsWith) {
  EXPECT_TRUE(EndsWith("foo bar baz", "z"));
  EXPECT_TRUE(EndsWith("foo bar baz", "baz"));
  EXPECT_TRUE(EndsWith("foo bar baz", " baz"));
  EXPECT_TRUE(EndsWith("foo bar baz", "bar baz"));
  EXPECT_TRUE(EndsWith("foo bar baz", "foo bar baz"));
  EXPECT_FALSE(EndsWith("foo bar baz", "1foo bar baz"));
}

TEST(StringUtilTest, StartsWith) {
  EXPECT_TRUE(StartsWith("foo bar baz", "f"));
  EXPECT_TRUE(StartsWith("foo bar baz", "foo"));
  EXPECT_TRUE(StartsWith("foo bar baz", "foo "));
  EXPECT_TRUE(StartsWith("foo bar baz", "foo bar"));
  EXPECT_TRUE(StartsWith("foo bar baz", "foo bar baz"));
  EXPECT_FALSE(StartsWith("foo bar baz", "foo bar baz1"));
}

TEST(StringUtilTest, Split) {
  std::vector<std::string> expected = {"ab", "cde", "f"};  
  EXPECT_EQ(expected, Split("ab cde f", " "));
  EXPECT_EQ(expected, Split("ab<>cde<>f", "<>"));
  EXPECT_EQ(expected, Split("ab -Icde -If", " -I"));
}

}  // namespace bdm
