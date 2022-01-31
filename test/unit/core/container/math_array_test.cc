// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#include "gtest/gtest.h"

#include "core/container/math_array.h"
#include "unit/test_util/io_test.h"

namespace bdm {

TEST(MathArray, DefaultConstructor) {
  MathArray<double, 3> a = {1, 2, 3};
  a = MathArray<double, 3>();

  EXPECT_NEAR(0.0, a[0], abs_error<double>::value);
  EXPECT_NEAR(0.0, a[1], abs_error<double>::value);
  EXPECT_NEAR(0.0, a[2], abs_error<double>::value);
}

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
  MathArray<double, 3> a;
  MathArray<double, 4> b{0, 0, 0, 0};
  MathArray<double, 4> c{1, 2, 3, 4};

  MathArray<double, 3> fill_result{1, 1, 1};
  MathArray<double, 4> entrywise_result{1, 4, 9, 16};

  a.fill(1);
  ASSERT_EQ(a, fill_result);

  ASSERT_EQ(a.Sum(), 3);

  EXPECT_NEAR(a.Norm(), 1.7320508075688772935, abs_error<double>::value);

  a = {1.1, 2.2, 3.3};
  a.Normalize();
  EXPECT_NEAR(0.2672612419124244187, a[0], abs_error<double>::value);
  EXPECT_NEAR(0.5345224838248488374, a[1], abs_error<double>::value);
  EXPECT_NEAR(0.8017837257372732561, a[2], abs_error<double>::value);

  ASSERT_EQ(b.Norm(), 0.0);

  ASSERT_EQ(c.EntryWiseProduct(c), entrywise_result);
}

// We expect that the GetNomalizedArray member function does not change the
// object itself but generates a new object whose
TEST(MathArray, GetNormalizedArray) {
  MathArray<double, 3> a{1.1, 2.2, 3.3};
  MathArray<double, 3> normalized{0.2672612419124244187, 0.5345224838248488374,
                                  0.8017837257372732561};
  MathArray<double, 3> d = a.GetNormalizedArray();
  double a_norm = a.Norm();

  // We dont't want to have the same object.
  ASSERT_NE(&d, &a);
  // We want array d to be noralized as our computed reference values.
  EXPECT_DOUBLE_EQ(normalized[0], d[0]);
  EXPECT_DOUBLE_EQ(normalized[1], d[1]);
  EXPECT_DOUBLE_EQ(normalized[2], d[2]);
  EXPECT_DOUBLE_EQ(1.0, d.Norm());
  // We don't want array a to change during that process.
  EXPECT_DOUBLE_EQ(1.1, a[0]);
  EXPECT_DOUBLE_EQ(2.2, a[1]);
  EXPECT_DOUBLE_EQ(3.3, a[2]);
  // We test if we can rescale back to array.
  d *= a_norm;
  EXPECT_DOUBLE_EQ(d[0], a[0]);
  EXPECT_DOUBLE_EQ(d[1], a[1]);
  EXPECT_DOUBLE_EQ(d[2], a[2]);
}

TEST(MathArray, NormalizeZeroVectorDeath) {
  EXPECT_DEATH_IF_SUPPORTED(
      {
        Double3 x({0.0, 0.0, 0.0});
        x.Normalize();
      },
      ".*You tried to normalize a zero vector..*");
}

#ifdef USE_DICT
TEST_F(IOTest, MathArray) {
  MathArray<double, 4> test{0.5, -1, 10, 500};
  MathArray<double, 4>* restored = nullptr;

  BackupAndRestore(test, &restored);
  for (size_t i = 0; i < 4; ++i) {
    ASSERT_EQ(test[i], (*restored)[i]);
  }

  delete restored;
}
#endif  // USE_DICT

}  // namespace bdm
