#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include <array>
#include <cmath>
#include <numeric>
#include <vector>
#include "matrix.h"

#include "random.h"

namespace bdm {

struct Math {
  /// value of pi
  static constexpr double kPi = 3.141592653589793238462643383279502884;

  template <typename T>
  static T Sum(const std::vector<T>& v) {
    return std::accumulate(v.begin(), v.end(), 0);
  }

  /// Returns the euclidean norm of a vector.
  /// @param a vector
  /// @return it's norm
  static double Norm(const std::array<double, 3>& a) {
    double norm = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (norm == 0.0) {
      norm = 1.0;
    }
    return norm;
  }

  /// Normalizes a vector.
  /// @param a a vector
  /// @return the vector divided by its norm
  static std::array<double, 3> Normalize(const std::array<double, 3>& a) {
    const auto&& norm = Norm(a);
    std::array<double, 3> ret;
    ret[0] = a[0] / norm;
    ret[1] = a[1] / norm;
    ret[2] = a[2] / norm;
    return ret;
  }

  // Helper function that returns distance (L2 norm) between two positions in 3D
  static double GetL2Distance(const std::array<double, 3>& pos1,
                              const std::array<double, 3>& pos2) {
    std::array<double, 3> dist_array;
    dist_array[0] = pos2[0] - pos1[0];
    dist_array[1] = pos2[1] - pos1[1];
    dist_array[2] = pos2[2] - pos1[2];
    return Norm(dist_array);
  }

  /// Returns the cross product of two vectors.
  /// @param a
  /// @param b
  /// @return result the cross product of a and b (a x b)
  template<std::size_t N>
  static std::array<double, N> CrossProduct(const std::array<double, N>& a, const std::array<double, N>& b) {
    std::array<double, N> result;
    result[0] = a[1] * b[2] - a[2] * b[1];
    result[1] = a[2] * b[0] - a[0] * b[2];
    result[2] = a[0] * b[1] - a[1] * b[0];
    return result;
  }

  /// Returns a vector of norm 1 perpendicular to a 3D vector. As usual there is
  /// no length check.
  /// @param a vector
  /// @return a perpendicular vector
  static std::array<double, 3> Perp3(const std::array<double, 3>& a, double random) {
    std::array<double, 3> vect_perp;
    if (a[0] == 0.0) {
      vect_perp[0] = 1.0;
      vect_perp[1] = 0.0;
      vect_perp[2] = 0.0;
      vect_perp = RotAroundAxis(vect_perp, 6.35 * random, a);
    } else {
      vect_perp[0] = a[1];
      vect_perp[1] = -a[0];
      vect_perp[2] = 0.0;
      vect_perp = Normalize(vect_perp);
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
  static std::array<double, 3> RotAroundAxis(const std::array<double, 3>& vector, double theta,
                                             const std::array<double, 3>& axis) {
    auto naxis = Normalize(axis);

    auto temp_1 = Matrix::ScalarMult(Matrix::Dot(vector, naxis), naxis);
    auto temp_2 = Matrix::ScalarMult(std::cos(-theta), Matrix::Subtract(vector, temp_1));
    auto temp_3 = Matrix::ScalarMult(std::sin(-theta), CrossProduct(vector, naxis));

    return {
      temp_1[0] + temp_2[0] + temp_3[0],
      temp_1[1] + temp_2[1] + temp_3[1],
      temp_1[2] + temp_2[2] + temp_3[2],
    };
  }

  /// Returns the angle (in radian) between two vectors.
  /// @param a the first vector
  /// @param b the second vector
  /// @return the angle between them.
  static double AngleRadian(const std::array<double, 3>& a, const std::array<double, 3>& b) {
    return std::acos(Matrix::Dot(a, b) / (Math::Norm(a) * Math::Norm(b)));
  }

  /// Returns the projection of the first vector onto the second one.
  /// @param a
  /// @param b
  /// @return the projection of a onto b
  static std::array<double, 3> ProjectionOnto(const std::array<double, 3>& a, const std::array<double, 3>& b) {
   double k = Matrix::Dot(a, b) / Matrix::Dot(b, b);
   return Matrix::ScalarMult(k, b);
 }
};

}  // namespace bdm

#endif  // MATH_UTIL_H_
