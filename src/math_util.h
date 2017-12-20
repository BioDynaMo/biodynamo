#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include <array>
#include <cmath>
#include <numeric>
#include <vector>

namespace bdm {

struct Math {
  /// value of pi
  static constexpr double kPi = 3.141592653589793238462643383279502884;

  static double Norm(const std::array<double, 3>& a) {
    double norm = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (norm == 0.0) {
      norm = 1.0;
    }
    return norm;
  }

  static std::array<double, 3> Normalize(const std::array<double, 3>& a) {
    auto norm = Norm(a);
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
};

}  // namespace bdm

#endif  // MATH_UTIL_H_
