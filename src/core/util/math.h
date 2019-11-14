// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_UTIL_MATH_H_
#define CORE_UTIL_MATH_H_

#include <array>
#include <cmath>
#include <numeric>
#include <vector>

#include <TMath.h>

#include "core/container/math_array.h"
#include "core/util/random.h"

namespace bdm {

struct Math {
  /// value of pi
  static constexpr double kPi = TMath::Pi();
  /// Helpful constant to identify 'infinity'
  static constexpr double kInfinity = 1e20;

  static double ToDegree(double rad) { return rad * (180 / kPi); }
  static double ToRadian(double deg) { return deg * (kPi / 180); }

  // Helper function that returns distance (L2 norm) between two positions in 3D
  static double GetL2Distance(const Double3& pos1, const Double3& pos2) {
    Double3 dist_array;
    dist_array[0] = pos2[0] - pos1[0];
    dist_array[1] = pos2[1] - pos1[1];
    dist_array[2] = pos2[2] - pos1[2];
    return dist_array.Norm();
  }

  /// Returns the cross product of two vectors.
  /// @param a
  /// @param b
  /// @return result the cross product of a and b (a x b)
  template <std::size_t N>
  static MathArray<double, N> CrossProduct(const MathArray<double, N>& a,
                                           const MathArray<double, N>& b) {
    MathArray<double, N> result;
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
    return result;
  }

  /// Returns a vector of norm 1 perpendicular to a 3D vector. As usual there is
  /// no length check.
  /// @param a vector
  /// @param random
  /// @return a perpendicular vector
  static Double3 Perp3(const Double3& a, double random) {
    Double3 vect_perp;
    if (a[0] == 0.0) {
      vect_perp[0] = 1.0;
      vect_perp[1] = 0.0;
      vect_perp[2] = 0.0;
      vect_perp = RotAroundAxis(vect_perp, 6.35 * random, a);
    } else {
      vect_perp[0] = a[1];
      vect_perp[1] = -a[0];
      vect_perp[2] = 0.0;
      vect_perp.Normalize();
      vect_perp = RotAroundAxis(vect_perp, 6.35 * random, a);
    }
    return vect_perp;
  }

  /// Performs a rotation of a 3D vector `vector` around a given axis `axis`,
  /// in the positive mathematical sens.
  ///
  /// @param[in] vector the vector we want to rotate
  /// @param[in] theta  the amplitude of rotation (in radian)
  /// @param[in] axis   (also a vector)
  /// @return the vector after rotation
  static Double3 RotAroundAxis(const Double3& vector, double theta,
                               const Double3& axis) {
    auto naxis = axis;
    naxis.Normalize();

    auto temp_1 = naxis * (vector * naxis);
    auto temp_2 = (vector - temp_1) * std::cos(-theta);
    auto temp_3 = CrossProduct(vector, naxis) * std::sin(-theta);

    return temp_1 + temp_2 + temp_3;
  }

  /// Returns the angle (in radian) between two vectors.
  /// @param a the first vector
  /// @param b the second vector
  /// @return the angle between them.
  static double AngleRadian(const Double3& a, const Double3& b) {
    return std::acos(a * b / (a.Norm() * b.Norm()));
  }

  /// Returns the projection of the first vector onto the second one.
  /// @param a
  /// @param b
  /// @return the projection of a onto b
  static Double3 ProjectionOnto(const Double3& a, const Double3& b) {
    double k = (a * b) / (b * b);
    return b * k;
  }
};

}  // namespace bdm

#endif  // CORE_UTIL_MATH_H_
