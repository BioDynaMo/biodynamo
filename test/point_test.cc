#include <gtest/gtest.h>
#include <stdlib.h>

#include "spatial/Point.h"

namespace bdm {

TEST(PointTest, LengthTest) {
  Point point1(0, 0, 0);
  Point point2(1, 1, 1);
  Point point3(2, 0, 2);

  ASSERT_DOUBLE_EQ(0, point1.Length());
  ASSERT_DOUBLE_EQ(sqrt(3), point2.Length());
  ASSERT_DOUBLE_EQ(2 * sqrt(2), point3.Length());
}

TEST(PointTest, DistanceTest) {
  Point point1(0, 0, 0);
  Point point2(1, 1, 1);
  Point point3(2, 0, 2);

  ASSERT_DOUBLE_EQ(sqrt(3), point1.EuclidianDistance(point2));
  ASSERT_DOUBLE_EQ(2 * sqrt(2), point1.EuclidianDistance(point3));
  ASSERT_DOUBLE_EQ(sqrt(3), point2.EuclidianDistance(point3));
}

TEST(PointTest, SqDistanceTest) {
  Point point1(0, 0, 0);
  Point point2(1, 1, 1);
  Point point3(2, 0, 2);

  ASSERT_DOUBLE_EQ(3, point1.SquaredEuclidianDistance(point2));
  ASSERT_DOUBLE_EQ(8, point1.SquaredEuclidianDistance(point3));
  ASSERT_DOUBLE_EQ(3, point2.SquaredEuclidianDistance(point3));
}

TEST(PointTest, ScolarMultiplicationTest) {
  Point point1(0, 0, 0);
  Point point2(1, 1, 1);
  Point point3(2, 0, 2);

  ASSERT_DOUBLE_EQ(0, point1 * point2);
  ASSERT_DOUBLE_EQ(0, point1 * point3);
  ASSERT_DOUBLE_EQ(4, point2 * point3);
  ASSERT_DOUBLE_EQ(0, point3 * point1);
}

TEST(PointTest, MultiplicationOnScolarTest) {
  Point point1(0, 0, 0);
  Point point2(1, 1, 1);
  Point point3(2, 0, 2);

  ASSERT_DOUBLE_EQ(0, (point1 * 5.5).x);
  ASSERT_DOUBLE_EQ(0, (point1 * 5.5).y);
  ASSERT_DOUBLE_EQ(0, (point1 * 5.5).z);
  ASSERT_DOUBLE_EQ(5.5, (point2 * 5.5).x);
  ASSERT_DOUBLE_EQ(5.5, (point2 * 5.5).y);
  ASSERT_DOUBLE_EQ(5.5, (point2 * 5.5).z);
  ASSERT_DOUBLE_EQ(11, (point3 * 5.5).x);
  ASSERT_DOUBLE_EQ(0, (point3 * 5.5).y);
  ASSERT_DOUBLE_EQ(11, (point3 * 5.5).z);
}

TEST(PointTest, AdditionTest) {
  Point point1(0, 0, 0);
  Point point2(1, 1, 1);
  Point point3(2, 0, 2);

  ASSERT_DOUBLE_EQ(1, (point1 + point2).x);
  ASSERT_DOUBLE_EQ(1, (point1 + point2).y);
  ASSERT_DOUBLE_EQ(1, (point1 + point2).z);
  ASSERT_DOUBLE_EQ(2, (point1 + point3).x);
  ASSERT_DOUBLE_EQ(0, (point1 + point3).y);
  ASSERT_DOUBLE_EQ(2, (point1 + point3).z);
  ASSERT_DOUBLE_EQ(3, (point2 + point3).x);
  ASSERT_DOUBLE_EQ(1, (point2 + point3).y);
  ASSERT_DOUBLE_EQ(3, (point2 + point3).z);
}

TEST(PointTest, EqualTest) {
  Point point1(0.5, 0.7, 0.9);
  Point point2(0.5, 0.7, 0.9);

  ASSERT_TRUE(point1.equals(point2));
}

}  // namespace bdm
