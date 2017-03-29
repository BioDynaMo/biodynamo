#include "backend.h"
#include "Vc/Vc"
#include "gtest/gtest.h"

namespace bdm {

TEST(ScalarVectorTest, All) {
  Vc::SimdArray<double, 1> scalar(3.14);
  EXPECT_DOUBLE_EQ(scalar[0], 3.14);
  auto sum = scalar + scalar;
  EXPECT_DOUBLE_EQ(sum[0], 6.28);
}

}  // namespace bdm
