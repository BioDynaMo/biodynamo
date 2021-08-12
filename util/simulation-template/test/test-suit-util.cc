// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include <gtest/gtest.h>
#include "my-simulation.h"

// Googletest in combination with the provided CMakeLists.txt allows you to
// define tests in arbitrary .cc files in the `test/` folder. We propose
// this file to test some general utilities. If you wish to add tests for
// other aspects, you can either add them to the existing test-*.cc files or
// create a new test-*.cc file in the test/folder. CMake will handle it
// automatically.

// Show how to compare two stings
TEST(UtilTest, StringTest) {
  // Expect two strings not to be equal
  EXPECT_STRNE("hello", "world");
}

// Show how to compare two numbers
TEST(UtilTest, NumberTest) {
  // Expect equality
  EXPECT_EQ(7 * 6, 42);
}

// Test if our function SquareMaxTemplate behaves as expected
TEST(UtilTest, SquareMaxFromSimTest) {
  // Expect equality for the following
  EXPECT_EQ(1.0, bdm::SquareMaxTemplate(1.0, 10.0));
  EXPECT_EQ(4.0, bdm::SquareMaxTemplate(2.0, 10.0));
  EXPECT_EQ(9.0, bdm::SquareMaxTemplate(3.0, 10.0));
  EXPECT_EQ(10.0, bdm::SquareMaxTemplate(4.0, 10.0));
  EXPECT_EQ(10.0, bdm::SquareMaxTemplate(5.0, 10.0));
}