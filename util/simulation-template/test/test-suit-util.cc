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

// Googletest in combination with the provided CMakeLists.txt allows you to
// define tests in arbitrary .cc files in the `test/` folder. We propose
// this file to test some general utilities.

// Example function `SquareMax`:
// Computes the square of `to_square` but output is bounded by `upper_bound`.
// Typically this function would occur somewhere in `src/` and we would include
// it here. For simplicity, we now define it here.
double SquareMax(double to_square, double upper_bound) {
  double square = to_square * to_square;
  if (square < upper_bound) {
    return square;
  } else {
    return upper_bound;
  }
}

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

// Test if our function SquareMax behaves as expected
TEST(UtilTest, SquareMaxTest) {
  // Expect equality for the following
  EXPECT_EQ(1.0, SquareMax(1.0, 10.0));
  EXPECT_EQ(4.0, SquareMax(2.0, 10.0));
  EXPECT_EQ(9.0, SquareMax(3.0, 10.0));
  EXPECT_EQ(10.0, SquareMax(4.0, 10.0));
  EXPECT_EQ(10.0, SquareMax(5.0, 10.0));
}