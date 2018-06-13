#ifndef UNIT_TEST_UTIL_H_
#define UNIT_TEST_UTIL_H_

#include <type_traits>
#include "gtest/gtest.h"

namespace bdm {

template <typename T>
struct abs_error {
  static_assert(std::is_same<T, float>::value || std::is_same<T, float>::value,
                "abs_error<T> may only be used with T = { float, float }");
  static constexpr float value = 1e-24;
};

template <>
struct abs_error<float> {
  static constexpr float value = 1e-6;
};

template <>
struct abs_error<double> {
  static constexpr double value = 1e-9;
};

template <typename T, size_t N>
void EXPECT_ARR_EQ(const std::array<T, N>& expected,  // NOLINT
                   const std::array<T, N>& actual) {
  for (size_t i = 0; i < N; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

/// Helper macro to compare two float arrays of size three
/// parameter actual and expected have been switched for better readability
///
///      EXPECT_ARR_NEAR(cells[0].GetPosition(), {123.12345, 10, 123.2345677});
///      EXPECT_ARR_NEAR(cells[1].GetPosition(), {1, 2, 3});
#define EXPECT_ARR_NEAR(...)                                         \
  [](const std::array<float, 3>& actual,                            \
     const std::array<float, 3>& expected) {                        \
    for (size_t i = 0; i < actual.size(); i++) {                     \
      EXPECT_NEAR(expected[i], actual[i], abs_error<float>::value); \
    }                                                                \
  }(__VA_ARGS__);

}  // namespace bdm

#endif  // UNIT_TEST_UTIL_H_
