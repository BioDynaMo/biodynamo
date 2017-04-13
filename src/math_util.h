#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include <array>

namespace bdm {

struct Math {
  /// value of pi
  static constexpr double kPi = 3.141592653589793238462643383279502884;

  static double Norm(const std::array<double, 3>& a) {
    double norm = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
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
};

}  // namespace bdm

#endif  // MATH_UTIL_H_
