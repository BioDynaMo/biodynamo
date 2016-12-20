#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

#include "backend.h"

namespace bdm {

struct Math {
  /// value of pi
  static constexpr double kPi = 3.141592653589793238462643383279502884;

  template <typename Backend>
  static typename Backend::real_v Norm(
      const std::array<typename Backend::real_v, 3>& a) {
    using real_v = typename Backend::real_v;
    real_v norm;
    for (size_t i = 0; i < real_v::Size; i++) {
      norm[i] = a[0][i] * a[0][i] + a[1][i] * a[1][i] + a[2][i] * a[2][i];
      if (norm[i] == 0.0) {
        norm[i] = 1.0;
      }
    }
    return norm;
  }

  template <typename Backend>
  static std::array<typename Backend::real_v, 3> Normalize(
      const std::array<typename Backend::real_v, 3>& a) {
    using real_v = typename Backend::real_v;
    auto norm = Norm<Backend>(a);
    std::array<real_v, 3> ret;
    ret[0] = a[0] / norm;
    ret[1] = a[1] / norm;
    ret[2] = a[2] / norm;
    return ret;
  }
};

}  // namespace bdm

#endif  // MATH_UTIL_H_