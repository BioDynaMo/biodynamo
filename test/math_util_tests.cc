#include <cmath>
#include "random.h"

#include "gtest/gtest.h"
#include "math_util.h"

namespace bdm {

TEST (MathUtilTest, exp) {
  double x = -0.0028724514195400627;
  double result = MathUtil::exp(x);
  double expected = 0.99713167012178527;
  ASSERT_DOUBLE_EQ(expected, result);
}

TEST (MathUtilTest, sqrt) {
  ASSERT_DOUBLE_EQ(std::sqrt(53.51089851859078), MathUtil::sqrt(53.51089851859078));
}

TEST (MathUtilTest, cbrt) {
  ASSERT_DOUBLE_EQ(std::cbrt(9), MathUtil::cbrt(9));
  ASSERT_DOUBLE_EQ(std::cbrt(53.51089851859078), MathUtil::cbrt(53.51089851859078));
}

TEST (MathUtilTest, cos) {
  ASSERT_DOUBLE_EQ(std::cos(53.51089851859078), MathUtil::cos(53.51089851859078));
}

TEST (MathUtilTest, sin) {
  ASSERT_DOUBLE_EQ(std::sin(53.51089851859078), MathUtil::sin(53.51089851859078));
}

TEST (MathUtilTest, acos) {
  ASSERT_DOUBLE_EQ(std::acos(0.51089851859078), MathUtil::acos(0.51089851859078));
}

TEST (MathUtilTest, asin) {
  ASSERT_DOUBLE_EQ(std::asin(0.51089851859078), MathUtil::asin(0.51089851859078));
}

TEST (MathUtilTest, atan2) {
  ASSERT_DOUBLE_EQ(std::atan2(0,0), MathUtil::atan2(0,0));
  ASSERT_DOUBLE_EQ(std::atan2(1,0), MathUtil::atan2(1,0));
  ASSERT_DOUBLE_EQ(std::atan2(-1,0), MathUtil::atan2(-1,0));
  ASSERT_DOUBLE_EQ(std::atan2(0,1), MathUtil::atan2(0,1));
  ASSERT_DOUBLE_EQ(std::atan2(0,-1), MathUtil::atan2(0,-1));
  ASSERT_DOUBLE_EQ(std::atan2(1,-1), MathUtil::atan2(1,-1));
  ASSERT_DOUBLE_EQ(std::atan2(-1,1), MathUtil::atan2(-1,1));
}

TEST (MathUtilTest, log) {
ASSERT_DOUBLE_EQ(std::log(53.51089851859078), MathUtil::log(53.51089851859078));
}

}
  // namespace bdm