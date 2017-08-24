#ifndef TEST_UTIL_H_
#define TEST_UTIL_H_

#include <type_traits>
#include "compile_time_param.h"
#include "gtest/gtest.h"

namespace bdm {

struct CompileTimeParam : public DefaultCompileTimeParam<> {};

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

template <typename T, size_t N>
void EXPECT_ARR_EQ(const std::array<T, N>& expected,  // NOLINT
                   const std::array<T, N>& actual) {
  for (size_t i = 0; i < N; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

}  // namespace bdm

#endif  // TEST_UTIL_H_
