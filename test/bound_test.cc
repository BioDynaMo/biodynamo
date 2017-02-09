#include <gtest/gtest.h>
#include <stdlib.h>

#include "spatial/bound.h"

namespace bdm {

TEST(BoundTest, VolumeTest) {
  bound b1(0, 0, 0, 1, 1, 1);
  bound b2(5.7, 3.2, 7.7, 11, 12, 13);

  ASSERT_DOUBLE_EQ(1, b1.volume());
  ASSERT_DOUBLE_EQ(247.192, b2.volume());
}

TEST(BoundTest, HasTest) {
  point p1(0, 1, 2);
  point p2(10, 10, 10);

  bound b1(0, 0, 0, 3, 3, 3);
  bound b2(7, 8, 9, 11, 12, 13);

  ASSERT_TRUE(b1.has(p1));
  ASSERT_FALSE(b1.has(p2));
  ASSERT_TRUE(b2.has(p2));
  ASSERT_FALSE(b2.has(p1));
}

TEST(BoundTest, AddPointTest) {
  bound b1(0, 0, 0, 1, 1, 1);
  point p1(2, 2, 2);

  bound b2 = b1.add_point(p1);

  ASSERT_DOUBLE_EQ(0, b2.left());
  ASSERT_DOUBLE_EQ(0, b2.bottom());
  ASSERT_DOUBLE_EQ(0, b2.far());
  ASSERT_DOUBLE_EQ(2, b2.right());
  ASSERT_DOUBLE_EQ(2, b2.top());
  ASSERT_DOUBLE_EQ(2, b2.near());
}

TEST(BoundTest, AddBoundTest) {
  bound b1(0, 0, 0, 1, 1, 1);
  bound b3(2, 2, 2, 3, 3, 3);

  bound b2 = b1.add_bound(b3);

  ASSERT_DOUBLE_EQ(0, b2.left());
  ASSERT_DOUBLE_EQ(0, b2.bottom());
  ASSERT_DOUBLE_EQ(0, b2.far());
  ASSERT_DOUBLE_EQ(3, b2.right());
  ASSERT_DOUBLE_EQ(3, b2.top());
  ASSERT_DOUBLE_EQ(3, b2.near());
}

TEST(BoundTest, DiffIfAddPointTest) {
  bound b1(0, 0, 0, 1, 1, 1);
  point p1(2, 2, 2);

  ASSERT_DOUBLE_EQ(7, b1.diff_if_add_point(p1));
}

TEST(BoundTest, SqdistTest) {
  pair<double, double> p1 = make_pair<double, double>(1, 1);
  pair<double, double> p2 = make_pair<double, double>(3, 4);
  bound b1(0, 0, 0, 0, 0, 0);

  ASSERT_DOUBLE_EQ(13, b1.sqdist(p1, p2));
}

TEST(BoundTest, isBetweenTest) {
  bound b1(0, 0, 0, 0, 0, 0);
  ASSERT_TRUE(b1.is_between(1, 0, 2));
  ASSERT_TRUE(b1.is_between(1, 2, 0));
  ASSERT_TRUE(b1.is_between(1, 1, 2));
}

TEST(BoundTest, SegmentDistanceTest) {
  bound b1(0, 0, 0, 0, 0, 0);
  ASSERT_DOUBLE_EQ(1, b1.distance_segments(0, 1, 3, 2));
  ASSERT_DOUBLE_EQ(2, b1.distance_segments(0, 1, -3, -2));
}

TEST(BoundTest, DistanceTest) {
  bound b1(0, 0, 0, 1, 1, 1);
  bound b2(2, 2, 2, 3, 3, 3);

  ASSERT_DOUBLE_EQ(3, b1.distance(b2));

  bound b3(0, 2, 2, 1, 3, 3);

  ASSERT_DOUBLE_EQ(2, b1.distance(b3));

  bound b4(0, 0, 2, 1, 1, 3);

  ASSERT_DOUBLE_EQ(1, b1.distance(b4));

  bound b5(0.5, 0.5, 0.5, 1.5, 1.5, 1.5);

  ASSERT_DOUBLE_EQ(0, b1.distance(b5));
}

TEST(BoundTest, HalfSurfaceTest) {
  bound b1(0, 0, 0, 3, 5, 7);
  ASSERT_DOUBLE_EQ(3 * 5 + 5 * 7 + 3 * 7, b1.half_surface_area());
}

}  // namespace bdm
