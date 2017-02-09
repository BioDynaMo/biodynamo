#include <gtest/gtest.h>
#include <stdlib.h>

#include "spatial/point.h"

namespace bdm {

TEST(PointTest, LengthTest) {
  point p1(0, 0, 0);
  point p2(1, 1, 1);
  point p3(2, 0, 2);

  ASSERT_DOUBLE_EQ(0, p1.length());
  ASSERT_DOUBLE_EQ(sqrt(3), p2.length());
  ASSERT_DOUBLE_EQ(2 * sqrt(2), p3.length());
}

TEST(PointTest, DistanceTest) {
  point p1(0, 0, 0);
  point p2(1, 1, 1);
  point p3(2, 0, 2);

  ASSERT_DOUBLE_EQ(sqrt(3), p1.distance(p2));
  ASSERT_DOUBLE_EQ(2 * sqrt(2), p1.distance(p3));
  ASSERT_DOUBLE_EQ(sqrt(3), p2.distance(p3));
}

TEST(PointTest, SqDistanceTest) {
  point p1(0, 0, 0);
  point p2(1, 1, 1);
  point p3(2, 0, 2);

  ASSERT_DOUBLE_EQ(3, p1.sqdist(p2));
  ASSERT_DOUBLE_EQ(8, p1.sqdist(p3));
  ASSERT_DOUBLE_EQ(3, p2.sqdist(p3));
}

TEST(PointTest, ScolarMultiplicationTest) {
  point p1(0, 0, 0);
  point p2(1, 1, 1);
  point p3(2, 0, 2);

  ASSERT_DOUBLE_EQ(0, p1 * p2);
  ASSERT_DOUBLE_EQ(0, p1 * p3);
  ASSERT_DOUBLE_EQ(4, p2 * p3);
  ASSERT_DOUBLE_EQ(0, p3 * p1);
}

TEST(PointTest, MultiplicationOnScolarTest) {
  point p1(0, 0, 0);
  point p2(1, 1, 1);
  point p3(2, 0, 2);

  ASSERT_DOUBLE_EQ(0, (p1 * 5.5).x);
  ASSERT_DOUBLE_EQ(0, (p1 * 5.5).y);
  ASSERT_DOUBLE_EQ(0, (p1 * 5.5).z);
  ASSERT_DOUBLE_EQ(5.5, (p2 * 5.5).x);
  ASSERT_DOUBLE_EQ(5.5, (p2 * 5.5).y);
  ASSERT_DOUBLE_EQ(5.5, (p2 * 5.5).z);
  ASSERT_DOUBLE_EQ(11, (p3 * 5.5).x);
  ASSERT_DOUBLE_EQ(0, (p3 * 5.5).y);
  ASSERT_DOUBLE_EQ(11, (p3 * 5.5).z);
}

TEST(PointTest, AdditionTest) {
  point p1(0, 0, 0);
  point p2(1, 1, 1);
  point p3(2, 0, 2);

  ASSERT_DOUBLE_EQ(1, (p1 + p2).x);
  ASSERT_DOUBLE_EQ(1, (p1 + p2).y);
  ASSERT_DOUBLE_EQ(1, (p1 + p2).z);
  ASSERT_DOUBLE_EQ(2, (p1 + p3).x);
  ASSERT_DOUBLE_EQ(0, (p1 + p3).y);
  ASSERT_DOUBLE_EQ(2, (p1 + p3).z);
  ASSERT_DOUBLE_EQ(3, (p2 + p3).x);
  ASSERT_DOUBLE_EQ(1, (p2 + p3).y);
  ASSERT_DOUBLE_EQ(3, (p2 + p3).z);
}

TEST(PointTest, EqualTest) {
  point p1(0.5, 0.7, 0.9);
  point p2(0.5, 0.7, 0.9);

  ASSERT_TRUE(p1.equals(p2));
}

}  // namespace bdm
