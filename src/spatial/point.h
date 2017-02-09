#ifndef BIODYNAMO_POINT_H
#define BIODYNAMO_POINT_H

#include <math.h>

// Helpful constant to compare with 0
double const eps = 1e-20;
// Helpful constant to identify 'infinity'
double const inf = 1e20;

// Class 'point' represents point in a 3-dementional space
class point {
 public:
  // Coordinates of the point
  double x, y, z;

  // Default constructior
  point() : x(0), y(0), z(0) {}

  point(double x, double y, double z) : x(x), y(y), z(z) {}

  double Length() { return sqrt(x * x + y * y + z * z); }

  void Set(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  // Euclidian distance from 'this' to point 'p'
  double EuclidianDistance(point p) { return (p + *this * (-1)).Length(); }

  // Squared euclidian distance from 'this' to point 'p'
  double SquaredEuclidianDistance(point p) {
    double dx = x - p.x;
    double dy = y - p.y;
    double dz = z - p.z;

    return dx * dx + dy * dy + dz * dz;
  }

  // Scolar multiplication of the points
  double operator*(point p) { return x * p.x + y * p.y + z * p.z; }

  // Point to scolar multiplication
  point operator*(double a) { return point(x * a, y * a, z * a); }

  // Addition of the points
  point operator+(point b) { return point(x + b.x, y + b.y, z + b.z); }

  void operator=(point b) {
    x = b.x;
    y = b.y;
    z = b.z;
  }

  // Check if two points is equal or not
  bool equals(point b) {
    return fabs(x - b.x) < eps && fabs(y - b.y) < eps && fabs(z - b.z) < eps;
  }
};

#endif  // BIODYNAMO_POINT_H
