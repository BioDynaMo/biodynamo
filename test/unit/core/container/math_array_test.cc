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

#include "gtest/gtest.h"

#include "core/container/math_array.h"
#include "unit/test_util/io_test.h"

namespace bdm {

TEST(MathArray, DefaultConstructor) {
  MathArray<real_t, 3> a = {1, 2, 3};
  a = MathArray<real_t, 3>();

  EXPECT_NEAR(0.0, a[0], abs_error<real_t>::value);
  EXPECT_NEAR(0.0, a[1], abs_error<real_t>::value);
  EXPECT_NEAR(0.0, a[2], abs_error<real_t>::value);
}

TEST(MathArray, element_access) {
  MathArray<real_t, 4> real_vector{0, 1, 2, 3};

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
  MathArray<real_t, 4> real_vector{0, 1, 2, 3};
  MathArray<real_t, 0> real_vector_empty;

  ASSERT_EQ(real_vector.size(), 4u);
  ASSERT_EQ(real_vector_empty.size(), 0u);

  ASSERT_FALSE(real_vector.empty());
  ASSERT_TRUE(real_vector_empty.empty());
}

TEST(MathArray, math_operators) {
  MathArray<real_t, 4> a{0, 1, 2, 3};
  MathArray<real_t, 4> b{0, 1, 2, 3};

  MathArray<real_t, 4> sum_result{0, 2, 4, 6};
  MathArray<real_t, 4> sub_result{0, 0, 0, 0};
  MathArray<real_t, 4> scalar_mult_result{0, 3, 6, 9};
  MathArray<real_t, 4> scalar_division_result{0, 0.5, 1, 1.5};
  MathArray<real_t, 4> increment_result{1, 2, 3, 4};
  MathArray<real_t, 4> decrement_result{-1, 0, 1, 2};

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
  MathArray<real_t, 3> a;
  MathArray<real_t, 4> b{0, 0, 0, 0};
  MathArray<real_t, 4> c{1, 2, 3, 4};

  MathArray<real_t, 3> fill_result{1, 1, 1};
  MathArray<real_t, 4> entrywise_result{1, 4, 9, 16};

  a.fill(1);
  ASSERT_EQ(a, fill_result);

  ASSERT_EQ(a.Sum(), 3);

  EXPECT_NEAR(a.Norm(), 1.7320508075688772935, abs_error<real_t>::value);

  a = {1.1, 2.2, 3.3};
  a.Normalize();
  EXPECT_NEAR(0.2672612419124244187, a[0], abs_error<real_t>::value);
  EXPECT_NEAR(0.5345224838248488374, a[1], abs_error<real_t>::value);
  EXPECT_NEAR(0.8017837257372732561, a[2], abs_error<real_t>::value);

  ASSERT_EQ(b.Norm(), 0.0);

  ASSERT_EQ(c.EntryWiseProduct(c), entrywise_result);
}

// We expect that the GetNormalizedArray member function does not change the
// object itself but generates a new object whose
TEST(MathArray, GetNormalizedArray) {
  MathArray<real_t, 3> a{1.1, 2.2, 3.3};
  MathArray<real_t, 3> normalized{0.2672612419124244187, 0.5345224838248488374,
                                  0.8017837257372732561};
  MathArray<real_t, 3> d = a.GetNormalizedArray();
  real_t a_norm = a.Norm();

  // We don't want to have the same object.
  ASSERT_NE(&d, &a);
  // We want array d to be normalized as our computed reference values.
  EXPECT_REAL_EQ(normalized[0], d[0]);
  EXPECT_REAL_EQ(normalized[1], d[1]);
  EXPECT_REAL_EQ(normalized[2], d[2]);
  EXPECT_REAL_EQ(real_t(1.0), d.Norm());
  // We don't want array a to change during that process.
  EXPECT_REAL_EQ(real_t(1.1), a[0]);
  EXPECT_REAL_EQ(real_t(2.2), a[1]);
  EXPECT_REAL_EQ(real_t(3.3), a[2]);
  // We test if we can rescale back to array.
  d *= a_norm;
  EXPECT_REAL_EQ(d[0], a[0]);
  EXPECT_REAL_EQ(d[1], a[1]);
  EXPECT_REAL_EQ(d[2], a[2]);
}

TEST(MathArray, NormalizeZeroVectorDeath) {
  EXPECT_DEATH_IF_SUPPORTED(
      {
        Real3 x({0.0, 0.0, 0.0});
        x.Normalize();
      },
      ".*You tried to normalize a zero vector..*");
}

#ifdef USE_DICT
TEST_F(IOTest, MathArray) {
  MathArray<real_t, 4> test{0.5, -1, 10, 500};
  MathArray<real_t, 4>* restored = nullptr;

  BackupAndRestore(test, &restored);
  for (size_t i = 0; i < 4; ++i) {
    ASSERT_EQ(test[i], (*restored)[i]);
  }

  delete restored;
}
#endif  // USE_DICT

}  // namespace bdm
