// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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
#include "core/util/log.h"
#include "core/util/random.h"

namespace bdm {

struct Math {
  /// value of pi
  static constexpr real kPi = TMath::Pi();
  /// Helpful constant to identify 'infinity'
  static constexpr real kInfinity = 1e20;

  static real ToDegree(real rad) { return rad * (180 / kPi); }
  static real ToRadian(real deg) { return deg * (kPi / 180); }

  // Helper function that returns distance (L2 norm) between two positions in 3D
  static real GetL2Distance(const Real3& pos1, const Real3& pos2) {
    Real3 dist_array;
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
  static MathArray<real, N> CrossProduct(const MathArray<real, N>& a,
                                           const MathArray<real, N>& b) {
    MathArray<real, N> result;
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
  static Real3 Perp3(const Real3& a, real random) {
    Real3 vect_perp;
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
  static Real3 RotAroundAxis(const Real3& vector, real theta,
                               const Real3& axis) {
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
  static real AngleRadian(const Real3& a, const Real3& b) {
    return std::acos(a * b / (a.Norm() * b.Norm()));
  }

  /// Returns the projection of the first vector onto the second one.
  /// @param a
  /// @param b
  /// @return the projection of a onto b
  static Real3 ProjectionOnto(const Real3& a, const Real3& b) {
    real k = (a * b) / (b * b);
    return b * k;
  }

  /// Returns the mean squared error between two vectors
  static real MSE(const std::vector<real>& v1,
                    const std::vector<real>& v2) {
    if (v1.size() != v2.size()) {
      Log::Fatal("Math::MSE", "vectors must have same length");
    }
    real error = 0;
    for (size_t i = 0; i < v1.size(); ++i) {
      auto diff = v2[i] - v1[i];
      error += diff * diff;
    }
    return error / v1.size();
  }
};

}  // namespace bdm

#endif  // CORE_UTIL_MATH_H_
