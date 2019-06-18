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

#include "core/container/math_array.h"
#include "gtest/gtest.h"

namespace bdm {

TEST(MathArray, element_access) {
  MathArray<double, 4> real_vector{0, 1, 2, 3};

  for (int i = 0; i < 4; i++) {
    ASSERT_EQ(real_vector.at(i), i);
    ASSERT_EQ(real_vector[i], i);
  }

  ASSERT_EQ(real_vector.front(), 0);
  ASSERT_EQ(real_vector.back(), 3);

  int i = 0;
  for (auto e : real_vector) {
    ASSERT_EQ(e, real_vector[i]);
    i++;
  }
}

TEST(MathArray, capacity) {
  MathArray<double, 4> real_vector{0, 1, 2, 3};
  MathArray<double, 0> real_vector_empty;

  ASSERT_EQ(real_vector.size(), 4u);
  ASSERT_EQ(real_vector_empty.size(), 0u);

  ASSERT_FALSE(real_vector.empty());
  ASSERT_TRUE(real_vector_empty.empty());
}

TEST(MathArray, math_operators) {
  MathArray<double, 4> a{0, 1, 2, 3};
  MathArray<double, 4> b{0, 1, 2, 3};

  MathArray<double, 4> sum_result{0, 2, 4, 6};
  MathArray<double, 4> sub_result{0, 0, 0, 0};
  MathArray<double, 4> scalar_mult_result{0, 3, 6, 9};
  MathArray<double, 4> scalar_division_result{0, 0.5, 1, 1.5};
  MathArray<double, 4> increment_result{1, 2, 3, 4};
  MathArray<double, 4> decrement_result{-1, 0, 1, 2};

  ASSERT_EQ(a + b, sum_result);

  ASSERT_EQ(a - b, sub_result);

  ASSERT_EQ(a * b, 14);

  ASSERT_EQ(a * 3, scalar_mult_result);

  ASSERT_EQ(a + 1, increment_result);

  ASSERT_EQ(a - 1, decrement_result);

  ASSERT_EQ(a / 2, scalar_division_result);

  a++;
  ASSERT_EQ(a, increment_result);

  b--;
  ASSERT_EQ(b, decrement_result);
}

TEST(MathArray, complex_operations) {
  MathArray<double, 4> a;
  MathArray<double, 4> b{0, 0, 0, 0};

  MathArray<double, 4> fill_result{1, 1, 1, 1};
  MathArray<double, 4> normalize_result{0.5, 0.5, 0.5, 0.5};

  a.fill(1);
  ASSERT_EQ(a, fill_result);

  ASSERT_EQ(a.Sum(), 4);

  ASSERT_EQ(a.Norm(), 2);

  ASSERT_EQ(a.Normalize(), normalize_result);

  ASSERT_EQ(b.Norm(), 1);
}
}  // namespace bdm
