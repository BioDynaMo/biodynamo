#ifndef SPATIAL_POINT_H_
#define SPATIAL_POINT_H_

#include <cmath>
#include "spatial/utils.h"

namespace bdm {
// Class 'point' represents point in a 3-dementional space
class Point {
 public:
  // Coordinates of the point
  double x, y, z;

  // Default constructior
  Point() : x(0), y(0), z(0) {}

  Point(double x, double y, double z) : x(x), y(y), z(z) {}

  double Length() { return sqrt(x * x + y * y + z * z); }

  void Set(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  // Euclidian distance from 'this' to point 'p'
  double EuclidianDistance(Point const &p) const {
    return (p + *this * (-1)).Length();
  }

  // Squared euclidian distance from 'this' to point 'p'
  double SquaredEuclidianDistance(Point const &p) const {
    double dx = x - p.x;
    double dy = y - p.y;
    double dz = z - p.z;

    return dx * dx + dy * dy + dz * dz;
  }

  // Scolar multiplication of the points
  double operator*(Point const &p) const { return x * p.x + y * p.y + z * p.z; }

  // Point to scolar multiplication
  Point operator*(double a) const { return Point(x * a, y * a, z * a); }

  // Addition of the points
  Point operator+(Point const &b) const {
    return Point(x + b.x, y + b.y, z + b.z);
  }

  void operator=(Point const &b) {
    x = b.x;
    y = b.y;
    z = b.z;
  }

  // Check if two points is equal or not
  bool equals(Point const &b) const {
    return fabs(x - b.x) < eps && fabs(y - b.y) < eps && fabs(z - b.z) < eps;
  }
};
}
#endif  // SPATIAL_POINT_H_
