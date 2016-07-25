#include <param.h>
#include "math_util.h"

#include "crlibm.h"

namespace bdm {

double MathUtil::exp(double d) {
  return exp_rn(d);
}

double MathUtil::cbrt(double d) {
  return pow_rn(d, 1.0 / 3);
}

double MathUtil::sqrt(double d) {
  return pow_rn(d, 1.0 / 2);
}

double MathUtil::cos(double d) {
  return cos_rn(d);
}

double MathUtil::sin(double d) {
  return sin_rn(d);
}

double MathUtil::asin(double d) {
  return asin_rn(d);
}

double MathUtil::acos(double d) {
  return acos_rn(d);
}

double MathUtil::atan2(double y, double x) {
  if (x > 0) {
    return atan_rn(y / x);
  } else if (x < 0 && y >= 0) {
    return atan_rn(y / x) + Param::kPi;
  } else if (x < 0 && y < 0) {
    return atan_rn(y / x) - Param::kPi;
  } else if (x == 0 && y > 0) {
    return Param::kPi / 2;
  } else if (x == 0 && y < 0) {
    return -Param::kPi / 2;
  } else {
    return 0;
  }
}

double MathUtil::log(double d) {
  return log_rn(d);
}

}  // namespace bdm
