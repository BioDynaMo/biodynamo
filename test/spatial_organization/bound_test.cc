#include <gtest/gtest.h>

#include "spatial_organization/bound.h"

namespace bdm {
namespace spatial_organization {

TEST(BoundTest, isBetweenTest) {
  Bound bound1(0, 0, 0, 0, 0, 0);
  ASSERT_TRUE(bound1.IsBetween(1, 0, 2));
  ASSERT_TRUE(bound1.IsBetween(1, 2, 0));
  ASSERT_TRUE(bound1.IsBetween(1, 1, 2));
}

TEST(BoundTest, DistanceTest) {
  Bound bound1(0, 0, 0, 1, 1, 1);
  Bound bound2(2, 2, 2, 3, 3, 3);

  ASSERT_DOUBLE_EQ(3, bound1.SquaredDistance(bound2));

  Bound bound3(0, 2, 2, 1, 3, 3);

  ASSERT_DOUBLE_EQ(2, bound1.SquaredDistance(bound3));

  Bound b4(0, 0, 2, 1, 1, 3);

  ASSERT_DOUBLE_EQ(1, bound1.SquaredDistance(b4));

  Bound b5(0.5, 0.5, 0.5, 1.5, 1.5, 1.5);

  ASSERT_DOUBLE_EQ(0, bound1.SquaredDistance(b5));
}

TEST(BoundTest, HalfSurfaceTest) {
  Bound bound1(0, 0, 0, 3, 5, 7);
  ASSERT_DOUBLE_EQ(3 * 5 + 5 * 7 + 3 * 7, bound1.HalfSurfaceArea());
}

}  // namespace spatial_organization
}  // namespace bdm
