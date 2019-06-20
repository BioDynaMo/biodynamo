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

#include "core/util/math.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

namespace bdm {

TEST(MathUtilTest, L2Distance) {
  Double3 vector1 = {0, 0, 0};
  Double3 vector2 = {1, 2, 3};
  auto result1 = Math::GetL2Distance(vector1, vector2);
  auto result2 = Math::GetL2Distance(vector2, vector1);

  EXPECT_NEAR(3.7416573867739413855, result1, abs_error<double>::value);
  EXPECT_NEAR(3.7416573867739413855, result2, abs_error<double>::value);
}

TEST(MathUtilTest, RotAroundAxis) {
  Double3 axis = {1.0, 1.0, 0.0};
  Double3 vector = {4, 5, 6};
  double theta = Math::kPi;

  auto&& result = Math::RotAroundAxis(vector, theta, axis);
  EXPECT_ARR_NEAR(result, {5, 4, -6});
}

TEST(MathUtilTest, Perp3) {
  Double3 vector = {4, 5, 6};
  double random = 1.1234;

  auto&& result = Math::Perp3(vector, random);
  EXPECT_ARR_NEAR(result, {0.83614150897258999, -0.010824848782715613,
                           -0.54840696532946365});
}

TEST(MathUtilTest, AngleRadian) {
  Double3 a = {1, 2, 3};
  Double3 b = {9, 8, 7};

  double result = Math::AngleRadian(a, b);
  EXPECT_NEAR(0.489306575615854, result, abs_error<double>::value);
}

TEST(MathUtilTest, ProjectionOnto) {
  Double3 a = {1, 2, 3};
  Double3 b = {9, 8, 7};

  auto result = Math::ProjectionOnto(a, b);
  EXPECT_ARR_NEAR(
      result, {{2.134020618556701, 1.8969072164948453, 1.6597938144329896}});
}

}  // namespace bdm
