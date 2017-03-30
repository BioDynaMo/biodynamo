#ifndef SPATIAL_BOUND_H_
#define SPATIAL_BOUND_H_

#include <cmath>
#include <utility>
#include "point.h"

namespace bdm {
using std::pair;
using std::make_pair;

/**
 * Class 'Bound' represents rectangular Bound of the node of the tree
 **/

class Bound {
 public:
  // Bounds
  // far left Bottom, Near Right Top
  Point far_left_bottom_point_, near_right_top_point_;

  // Default constructior
  Bound() {
    far_left_bottom_point_ = Point(0, 0, 0);
    near_right_top_point_ = Point(1, 1, 1);
  }

  Bound(double x1, double y1, double z1, double x2, double y2, double z2) {
    far_left_bottom_point_ = Point(x1, y1, z1);
    near_right_top_point_ = Point(x2, y2, z2);
  }

  Bound(Point p1, Point p2)
      : far_left_bottom_point_(p1), near_right_top_point_(p2) {}

  // Check if 'x' is between 'a' and 'b' on the line
  bool IsBetween(double x, double a, double b) const {
    double min = fmin(a, b);
    double max = fmax(a, b);
    return (x >= min && x <= max);
  }

  // Calculate distance between two line segments on the line.
  // This method assumed that segments are not overlaped.
  double DistanceBetweenSegments(double x, double y, double a, double b) const {
    double min_xy = fmin(x, y);
    double max_xy = fmax(x, y);
    double min_ab = fmin(a, b);
    double max_ab = fmax(a, b);

    if (min_xy >= max_ab) return min_xy - max_ab;
    return min_ab - max_xy;
  }

  // Calculate squared distance between two boundaries in 3-d space.
  double SquaredDistance(Bound const &b) const {
    bool is_overlap_x;
    bool is_overlap_y;
    bool is_overlap_z;

    double bx[2][2] = {{Far(), Near()}, {b.Far(), b.Near()}};
    double by[2][2] = {{Left(), Right()}, {b.Left(), b.Right()}};
    double bz[2][2] = {{Bottom(), Top()}, {b.Bottom(), b.Top()}};

    // check axis_ that have overlaped projections
    is_overlap_x = (IsBetween(bx[0][0], bx[1][0], bx[1][1])) ||
                   (IsBetween(bx[0][1], bx[1][0], bx[1][1])) ||
                   (IsBetween(bx[1][0], bx[0][0], bx[0][1])) ||
                   (IsBetween(bx[1][1], bx[0][0], bx[0][1]));
    is_overlap_y = (IsBetween(by[0][0], by[1][0], by[1][1])) ||
                   (IsBetween(by[0][1], by[1][0], by[1][1])) ||
                   (IsBetween(by[1][0], by[0][0], by[0][1])) ||
                   (IsBetween(by[1][1], by[0][0], by[0][1]));
    is_overlap_z = (IsBetween(bz[0][0], bz[1][0], bz[1][1])) ||
                   (IsBetween(bz[0][1], bz[1][0], bz[1][1])) ||
                   (IsBetween(bz[1][0], bz[0][0], bz[0][1])) ||
                   (IsBetween(bz[1][1], bz[0][0], bz[0][1]));

    double dx = 0, dy = 0, dz = 0;

    // calculate distance only if there is no overlaping
    if (!is_overlap_x)
      dx = DistanceBetweenSegments(bx[0][0], bx[0][1], bx[1][0], bx[1][1]);

    if (!is_overlap_y)
      dy = DistanceBetweenSegments(by[0][0], by[0][1], by[1][0], by[1][1]);

    if (!is_overlap_z)
      dz = DistanceBetweenSegments(bz[0][0], bz[0][1], bz[1][0], bz[1][1]);
    return dx * dx + dy * dy + dz * dz;
  }

  Point Center() const {
    return (far_left_bottom_point_ + near_right_top_point_) * 0.5;
  }

  double Near() const { return near_right_top_point_.x_; }

  double Far() const { return far_left_bottom_point_.x_; }

  double Left() const { return far_left_bottom_point_.y_; }

  double Right() const { return near_right_top_point_.y_; }

  double Top() const { return near_right_top_point_.z_; }

  double Bottom() const { return far_left_bottom_point_.z_; }

  double Length() const {
    return near_right_top_point_.x_ - far_left_bottom_point_.x_;
  }

  double Width() const {
    return near_right_top_point_.y_ - far_left_bottom_point_.y_;
  }

  double Height() const {
    return near_right_top_point_.z_ - far_left_bottom_point_.z_;
  }

  double HalfSurfaceArea() const {
    return Width() * Length() + Height() * Length() + Width() * Height();
  }
};
}  // namespace bdm
#endif  // SPATIAL_BOUND_H_
