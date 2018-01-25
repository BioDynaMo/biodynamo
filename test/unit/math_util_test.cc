#include "math_util.h"
#include "backend.h"
#include "gtest/gtest.h"
#include "unit/test_util.h"

#include <fstream>

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

TEST(MathUtilTest, GaussianArray) {
  gTRandom.SetSeed(4357);
  int length = 180;
  auto arr = Math::CreateGaussianArray(90, 5, length);
  std::ofstream outfile;
  outfile.open ("gaussian_distribution.csv");
  for (int i = 0; i < length; i++) {
    outfile << arr[i] << std::endl;
  }
  outfile.close();
}

}  // namespace bdm
