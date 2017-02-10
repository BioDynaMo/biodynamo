#include <gtest/gtest.h>
#include <stdlib.h>

#include "spatial/bound.h"

namespace bdm {

TEST(BoundTest, VolumeTest) {
  bound bound_1(0, 0, 0, 1, 1, 1);
  bound bound_2(5.7, 3.2, 7.7, 11, 12, 13);

  ASSERT_DOUBLE_EQ(1, bound_1.Volume());
  ASSERT_DOUBLE_EQ(247.192, bound_2.Volume());
}

TEST(BoundTest, HasTest) {
  point point1(0, 1, 2);
  point point2(10, 10, 10);

  bound bound1(0, 0, 0, 3, 3, 3);
  bound bound2(7, 8, 9, 11, 12, 13);

  ASSERT_TRUE(bound1.Has(point1));
  ASSERT_FALSE(bound1.Has(point2));
  ASSERT_TRUE(bound2.Has(point2));
  ASSERT_FALSE(bound2.Has(point1));
}

TEST(BoundTest, AddPointTest) {
  bound bound1(0, 0, 0, 1, 1, 1);
  point point1(2, 2, 2);

  bound bound2 = bound1.AddPoint(point1);

  ASSERT_DOUBLE_EQ(0, bound2.Left());
  ASSERT_DOUBLE_EQ(0, bound2.Bottom());
  ASSERT_DOUBLE_EQ(0, bound2.Far());
  ASSERT_DOUBLE_EQ(2, bound2.Right());
  ASSERT_DOUBLE_EQ(2, bound2.Top());
  ASSERT_DOUBLE_EQ(2, bound2.Near());
}

TEST(BoundTest, AddBoundTest) {
  bound bound1(0, 0, 0, 1, 1, 1);
  bound bound3(2, 2, 2, 3, 3, 3);

  bound bound2 = bound1.AddBound(bound3);

  ASSERT_DOUBLE_EQ(0, bound2.Left());
  ASSERT_DOUBLE_EQ(0, bound2.Bottom());
  ASSERT_DOUBLE_EQ(0, bound2.Far());
  ASSERT_DOUBLE_EQ(3, bound2.Right());
  ASSERT_DOUBLE_EQ(3, bound2.Top());
  ASSERT_DOUBLE_EQ(3, bound2.Near());
}

TEST(BoundTest, DiffIfAddPointTest) {
  bound bound1(0, 0, 0, 1, 1, 1);
  point point1(2, 2, 2);

  ASSERT_DOUBLE_EQ(7, bound1.DifferenceOnBoundExtension(point1));
}

TEST(BoundTest, SqdistTest) {
  pair<double, double> point1 = make_pair<double, double>(1, 1);
  pair<double, double> point2 = make_pair<double, double>(3, 4);
  bound bound1(0, 0, 0, 0, 0, 0);

  ASSERT_DOUBLE_EQ(13, bound1.SquaredDistance(point1, point2));
}

TEST(BoundTest, isBetweenTest) {
  bound bound1(0, 0, 0, 0, 0, 0);
  ASSERT_TRUE(bound1.IsBetween(1, 0, 2));
  ASSERT_TRUE(bound1.IsBetween(1, 2, 0));
  ASSERT_TRUE(bound1.IsBetween(1, 1, 2));
}

TEST(BoundTest, SegmentDistanceTest) {
  bound bound1(0, 0, 0, 0, 0, 0);
  ASSERT_DOUBLE_EQ(1, bound1.DistanceBetweenSegments(0, 1, 3, 2));
  ASSERT_DOUBLE_EQ(2, bound1.DistanceBetweenSegments(0, 1, -3, -2));
}

TEST(BoundTest, DistanceTest) {
  bound bound1(0, 0, 0, 1, 1, 1);
  bound bound2(2, 2, 2, 3, 3, 3);

  ASSERT_DOUBLE_EQ(3, bound1.Distance(bound2));

  bound bound3(0, 2, 2, 1, 3, 3);

  ASSERT_DOUBLE_EQ(2, bound1.Distance(bound3));

  bound b4(0, 0, 2, 1, 1, 3);

  ASSERT_DOUBLE_EQ(1, bound1.Distance(b4));

  bound b5(0.5, 0.5, 0.5, 1.5, 1.5, 1.5);

  ASSERT_DOUBLE_EQ(0, bound1.Distance(b5));
}

TEST(BoundTest, HalfSurfaceTest) {
  bound bound1(0, 0, 0, 3, 5, 7);
  ASSERT_DOUBLE_EQ(3 * 5 + 5 * 7 + 3 * 7, bound1.HalfSurfaceArea());
}

}  // namespace bdm
