#ifndef SRC_SPATIAL_UTILS_H_
#define SRC_SPATIAL_UTILS_H_

#include <cmath>
#include <utility>
#include "point.h"

using std::pair;
using std::make_pair;

// Class 'bound' represents rectangular bound of the node of the tree
struct bound {
  // Bounds
  // far left Bottom, Near Right Top
  point flb, nrt;

  // Default constructior
  bound() {
    flb = point(0, 0, 0);
    nrt = point(1, 1, 1);
  }

  bound(double x1, double y1, double z1, double x2, double y2, double z2) {
    flb = point(x1, y1, z1);
    nrt = point(x2, y2, z2);
  }

  bound(point p1, point p2) : flb(p1), nrt(p2) {}

  // The Volume of the rectangle
  double Volume() {
    return (Near() - Far()) * (Right() - Left()) * (Top() - Bottom());
  }

  // Checks if point 'p' lies inside the bound or on its boundary
  bool Has(point p) {
    return IsBetween(p.x, Far(), Near()) && IsBetween(p.y, Left(), Right()) &&
            IsBetween(p.z, Bottom(), Top());
  }

  // Extends boundary so it contains point 'p'
  bound AddPoint(point p) {
    bound new_bnd(fmin(p.x, Far()), fmin(p.y, Left()), fmin(p.z, Bottom()),
                  fmax(p.x, Near()), fmax(p.y, Right()), fmax(p.z, Top()));
    return new_bnd;
  }

  // Extends boundary so it contains bound 'b'
  bound AddBound(bound b) {
    return bound(fmin(Far(), b.Far()), fmin(Left(), b.Left()),
                 fmin(Bottom(), b.Bottom()), fmax(Near(), b.Near()),
                 fmax(Right(), b.Right()), fmax(Top(), b.Top()));
  }

  // Calculate Volume difference between original
  double DifferenceOnBoundExtension(point p) { return AddPoint(p).Volume() - Volume(); }

  // Squared distance between two points on a plane
  double SquaredDistance(pair<double, double> p1, pair<double, double> p2) {
    double dx = p1.first - p2.first;
    double dy = p1.second - p2.second;
    return dx * dx + dy * dy;
  }

  // Check if 'x' is between 'a' and 'b' on the line
  bool IsBetween(double x, double a, double b) {
    double min = fmin(a, b);
    double max = fmax(a, b);
    return (x >= min && x <= max);
  }

  // Calculate distance between two line segments on the line.
  // This method assumed that segments are not overlaped.
  double DistanceBetweenSegments(double x, double y, double a, double b) {
    double minxy = fmin(x, y);
    double maxxy = fmax(x, y);
    double minab = fmin(a, b);
    double maxab = fmax(a, b);

    if (minxy >= maxab) return minxy - maxab;
    return minab - maxxy;
  }

  // Calculate squared distance between two boundaries in 3-d space.
  double Distance(bound b) {
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

  point Center() { return (flb + nrt) * 0.5; }

  double Near() { return nrt.x; }

  double Far() { return flb.x; }

  double Left() { return flb.y; }

  double Right() { return nrt.y; }

  double Top() { return nrt.z; }

  double Bottom() { return flb.z; }

  double Length() { return nrt.x - flb.x; }

  double Width() { return nrt.y - flb.y; }

  double Height() { return nrt.z - flb.z; }

  double HalfSurfaceArea() {
    return Width() * Length() + Height() * Length() + Width() * Height();
  }
};

#endif /* SRC_SPATIAL_UTILS_H_ */
