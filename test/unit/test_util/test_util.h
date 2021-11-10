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

#ifndef UNIT_TEST_UTIL_TEST_UTIL_H_
#define UNIT_TEST_UTIL_TEST_UTIL_H_

#include <type_traits>
#include "gtest/gtest.h"

#include "core/container/math_array.h"

namespace bdm {

// -----------------------------------------------------------------------------
template <typename T>
struct abs_error {
  static_assert(std::is_same<T, double>::value || std::is_same<T, float>::value,
                "abs_error<T> may only be used with T = { float, double }");
  static constexpr double value = 1e-24;
};

template <>
struct abs_error<float> {
  static constexpr double value = 1e-6;
};

template <>
struct abs_error<double> {
  static constexpr double value = 1e-9;
};

// -----------------------------------------------------------------------------
template <typename T, size_t N>
void EXPECT_ARR_EQ(const MathArray<T, N>& expected,  // NOLINT
                   const MathArray<T, N>& actual) {
  for (size_t i = 0; i < N; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

// TODO: this will be removed once the transition to MathArray is completed.
template <typename T, size_t N>
void EXPECT_ARR_EQ(const std::array<T, N>& expected,  // NOLINT
                   const std::array<T, N>& actual) {
  for (size_t i = 0; i < N; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

// -----------------------------------------------------------------------------
/// Helper macro to compare two double arrays of size three
/// parameter actual and expected have been switched for better readability
///
///      EXPECT_ARR_NEAR(cells[0].GetPosition(), {123.12345, 10, 123.2345677});
///      EXPECT_ARR_NEAR(cells[1].GetPosition(), {1, 2, 3});
#define EXPECT_ARR_NEAR(...)                                         \
  [](const Double3& actual, const Double3& expected) {               \
    for (size_t i = 0; i < actual.size(); i++) {                     \
      EXPECT_NEAR(expected[i], actual[i], abs_error<double>::value); \
    }                                                                \
  }(__VA_ARGS__);

#define EXPECT_VEC_NEAR(...)                                                   \
  [](const std::vector<double>& actual, const std::vector<double>& expected) { \
    for (size_t i = 0; i < actual.size(); i++) {                               \
      EXPECT_NEAR(expected[i], actual[i], abs_error<double>::value);           \
    }                                                                          \
  }(__VA_ARGS__);

#define EXPECT_ARR_NEAR_GPU(...)                       \
  [](const Double3& actual, const Double3& expected) { \
    for (size_t i = 0; i < actual.size(); i++) {       \
      EXPECT_NEAR(expected[i], actual[i], 1e-8);       \
    }                                                  \
  }(__VA_ARGS__);

#define EXPECT_ARR_NEAR4(...)                                        \
  [](const Double4& actual, const Double4& expected) {               \
    for (size_t i = 0; i < actual.size(); i++) {                     \
      EXPECT_NEAR(expected[i], actual[i], abs_error<double>::value); \
    }                                                                \
  }(__VA_ARGS__);

// -----------------------------------------------------------------------------
/// Mangled test name.\n
/// Only works within test class, since the implementation relies on `this`.
#define TEST_NAME typeid(*this).name()

// -----------------------------------------------------------------------------
/// This macro launches a given function in a new process.
/// The test will pass, if the function exits with exit code 0.
/// e.g. using `exit(0)`.
/// This is necessary for ParaView insitu tests because they invoke MPI_INIT
/// and MPI_FINALIZE. These two methods must not be called more than once per
/// process otherwise they will throw an error.
#define LAUNCH_IN_NEW_PROCESS(...) \
  EXPECT_EXIT(__VA_ARGS__, ::testing::ExitedWithCode(0), "");

}  // namespace bdm

#endif  // UNIT_TEST_UTIL_TEST_UTIL_H_
