#include "math_util.h"
#include "backend.h"
#include "gtest/gtest.h"
#include "test_util.h"

namespace bdm {

TEST(MathUtilTest, Norm) {
  using real_v = VcVectorBackend::real_v;
  std::array<real_v, 3> matrix = {real_v(0), real_v(0), real_v(0)};
  matrix[0][0] = 1.1;
  matrix[1][0] = 2.2;
  matrix[2][0] = 3.3;
  auto result = Math::Norm<VcVectorBackend>(matrix);

  EXPECT_NEAR(result[0], 16.94, abs_error<real_v::value_type>::value);
}

TEST(MathUtilTest, NormZero) {
  using real_v = VcVectorBackend::real_v;
  std::array<real_v, 3> matrix = {real_v(0), real_v(0), real_v(0)};
  auto result = Math::Norm<VcVectorBackend>(matrix);

  EXPECT_NEAR(result[0], 1, abs_error<real_v::value_type>::value);
}

TEST(MathUtilTest, NormalizeZero) {
  using real_v = VcVectorBackend::real_v;
  std::array<real_v, 3> matrix = {real_v(0), real_v(0), real_v(0)};
  auto result = Math::Normalize<VcVectorBackend>(matrix);

  EXPECT_NEAR(result[0].sum(), 0, abs_error<real_v::value_type>::value);
  EXPECT_NEAR(result[1].sum(), 0, abs_error<real_v::value_type>::value);
  EXPECT_NEAR(result[2].sum(), 0, abs_error<real_v::value_type>::value);
}

TEST(MathUtilTest, Normalize) {
  using real_v = VcVectorBackend::real_v;
  real_v vector(0.0);
  std::array<real_v, 3> matrix = {real_v(0), real_v(0), real_v(0)};
  matrix[0][0] = 1.1;
  matrix[1][0] = 2.2;
  matrix[2][0] = 3.3;
  auto result = Math::Normalize<VcVectorBackend>(matrix);

  EXPECT_NEAR(result[0][0], 0.0649350649350649351,
              abs_error<real_v::value_type>::value);
  EXPECT_NEAR(result[1][0], 0.1298701298701298701,
              abs_error<real_v::value_type>::value);
  EXPECT_NEAR(result[2][0], 0.1948051948051948052,
              abs_error<real_v::value_type>::value);
}

}  // namespace bdm
