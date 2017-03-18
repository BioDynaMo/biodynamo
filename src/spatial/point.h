#ifndef SPATIAL_POINT_H_
#define SPATIAL_POINT_H_

#include <cmath>
#include "spatial/utils.h"

namespace bdm {
// Class 'point' represents point in a 3-dementional space
class Point {
 public:
  // Coordinates of the point
  double x_, y_, z_;

  // Default constructior
  Point() : x_(0), y_(0), z_(0) {}

  Point(double x, double y, double z) : x_(x), y_(y), z_(z) {}

  double Length() { return sqrt(x_ * x_ + y_ * y_ + z_ * z_); }

  void Set(double x, double y, double z) {
    this->x_ = x;
    this->y_ = y;
    this->z_ = z;
  }

  // Squared euclidian distance from 'this' to point 'p'
  double SquaredEuclidianDistance(Point const &p) const {
    double dx = x_ - p.x_;
    double dy = y_ - p.y_;
    double dz = z_ - p.z_;

    return dx * dx + dy * dy + dz * dz;
  }
  // Euclidian distance from 'this' to point 'p'
  double EuclidianDistance(Point const &p) const {
    return (p + *this * (-1)).Length();
  }

  // Scalar multiplication of the points
  double operator*(Point const &p) const {
    return x_ * p.x_ + y_ * p.y_ + z_ * p.z_;
  }

  // Point to scolar multiplication
  Point operator*(double a) const { return Point(x_ * a, y_ * a, z_ * a); }

  // Addition of the points
  Point operator+(Point const &b) const {
    return Point(x_ + b.x_, y_ + b.y_, z_ + b.z_);
  }

  // Comparison of the points
  bool operator==(Point const &b) const { return equals(b); }

  // Comparison of the points
  bool operator!=(Point const &b) const { return !equals(b); }

  void operator=(Point const &b) {
    x_ = b.x_;
    y_ = b.y_;
    z_ = b.z_;
  }

  // Check if two points is equal or not
  bool equals(Point const &b) const {
    return fabs(x_ - b.x_) < kEpsilon && fabs(y_ - b.y_) < kEpsilon &&
           fabs(z_ - b.z_) < kEpsilon;
  }
};
}
#endif  // SPATIAL_POINT_H_
