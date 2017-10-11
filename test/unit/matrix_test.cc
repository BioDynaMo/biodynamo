#include "matrix.h"
#include <gtest/gtest.h>
#include "unit/test_util.h"

namespace bdm {

TEST(MatrixTest, Add) {
  array<double, 3> a = {0.5, 0.7, 1.2};
  array<double, 3> b = {0.6, 1.5, 2.1};
  auto result = Matrix::Add(a, b);

  EXPECT_NEAR(1.1, result[0], abs_error<double>::value);
  EXPECT_NEAR(2.2, result[1], abs_error<double>::value);
  EXPECT_NEAR(3.3, result[2], abs_error<double>::value);
}

TEST(MatrixTest, Subtract) {
  array<double, 3> a = {0.6, 1.5, 2.1};
  array<double, 3> b = {0.5, 0.7, 0.8};
  auto result = Matrix::Subtract(a, b);

  EXPECT_NEAR(0.1, result[0], abs_error<double>::value);
  EXPECT_NEAR(0.8, result[1], abs_error<double>::value);
  EXPECT_NEAR(1.3, result[2], abs_error<double>::value);
}

TEST(MatrixTest, Dot) {
  array<double, 3> a = {0.5, 0.7, 0.8};
  array<double, 3> b = {0.6, 1.5, 2.1};
  double result = Matrix::Dot(a, b);

  EXPECT_NEAR(3.03, result, abs_error<double>::value);
}

TEST(MatrixTest, ScalarMult) {
  array<double, 3> a = {0.5, 0.7, 0.8};
  double k = 3.2;
  auto result = Matrix::ScalarMult(k, a);

  EXPECT_ARR_NEAR(result, {1.6, 2.24, 2.56});
}

}  // namespace bdm
