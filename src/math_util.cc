#include "math_util.h"

#include <cmath>

namespace bdm {

double MathUtil::exp(double d) {
  return std::exp(d);
}

double MathUtil::cbrt(double d) {
  return std::cbrt(d);
}

double MathUtil::sqrt(double d) {
  return std::sqrt(d);
}

double MathUtil::cos(double d) {
  return std::cos(d);
}

double MathUtil::sin(double d) {
  return std::sin(d);
}

double MathUtil::asin(double d) {
  return std::asin(d);
}

double MathUtil::acos(double d) {
  return std::acos(d);
}

double MathUtil::atan2(double d, double d1) {
  return std::atan2(d, d1);
}

}  // namespace bdm
