#include "math_util.h"
#include "backend.h"
#include "gtest/gtest.h"
#include "unit/test_util.h"

namespace bdm {

TEST(MathUtilTest, Norm) {
  std::array<double, 3> vector = {1.1, 2.2, 3.3};
  auto result = Math::Norm(vector);

  EXPECT_NEAR(4.115823125451335, result, abs_error<double>::value);
}

TEST(MathUtilTest, NormZero) {
  std::array<double, 3> vector = {0, 0, 0};
  auto result = Math::Norm(vector);

  EXPECT_NEAR(1, result, abs_error<double>::value);
}

TEST(MathUtilTest, NormalizeZero) {
  std::array<double, 3> vector = {0, 0, 0};
  auto result = Math::Normalize(vector);

  EXPECT_NEAR(0, result[0], abs_error<double>::value);
  EXPECT_NEAR(0, result[1], abs_error<double>::value);
  EXPECT_NEAR(0, result[2], abs_error<double>::value);
}

TEST(MathUtilTest, Normalize) {
  std::array<double, 3> vector = {1.1, 2.2, 3.3};
  auto result = Math::Normalize(vector);

  EXPECT_NEAR(0.2672612419124244187, result[0], abs_error<double>::value);
  EXPECT_NEAR(0.5345224838248488374, result[1], abs_error<double>::value);
  EXPECT_NEAR(0.8017837257372732561, result[2], abs_error<double>::value);
}

TEST(MathUtilTest, L2Distance) {
  std::array<double, 3> vector1 = {0, 0, 0};
  std::array<double, 3> vector2 = {1, 2, 3};
  auto result1 = Math::GetL2Distance(vector1, vector2);
  auto result2 = Math::GetL2Distance(vector2, vector1);

  EXPECT_NEAR(3.7416573867739413855, result1, abs_error<double>::value);
  EXPECT_NEAR(3.7416573867739413855, result2, abs_error<double>::value);
}

TEST(MathUtilTest, Sum) {
  std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto result = Math::Sum(v);

  EXPECT_EQ(55, result);
}

TEST(MathUtilTest, CrossProduct) {
  std::array<double, 3> a = {1.1, 2.2, 3.3};
  std::array<double, 3> b = {5.8, 7.3, 11.87};

  auto&& result = Math::CrossProduct(a, b);
  EXPECT_ARR_NEAR(result, {2.024, 6.083, -4.73});
}

TEST(MathUtilTest, RotAroundAxis) {
  std::array<double, 3> axis = {1.0,1.0,0.0};
  std::array<double, 3> vector = {4,5,6};
  double theta = Math::kPi;

  auto&& result = Math::RotAroundAxis(vector, theta, axis);
  EXPECT_ARR_NEAR(result, {5, 4, -6});
}

TEST(MathUtilTest, Perp3) {
  std::array<double, 3> vector = {4,5,6};
  double random = 1.1234;

  auto&& result = Math::Perp3(vector, random);
  EXPECT_ARR_NEAR(result, {0.83614150897258999, -0.010824848782715613, -0.54840696532946365});
}

TEST(MathUtilTest, AngleRadian) {
  std::array<double, 3> a = {1,2,3};
  std::array<double, 3> b = {9,8,7};

  double result = Math::AngleRadian(a, b);
  EXPECT_NEAR(0.489306575615854, result, abs_error<double>::value);
}

}  // namespace bdm
