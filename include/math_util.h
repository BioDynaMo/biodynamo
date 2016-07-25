#ifndef MATH_UTIL_H_
#define MATH_UTIL_H_

namespace bdm {

/**
 * wrapper for Math functions
 */
struct MathUtil {
  static double exp(double d);

  static double cbrt(double d);

  static double sqrt(double d);

  static double cos(double d);

  static double sin(double d);

  static double asin(double d);

  static double acos(double d);

  static double atan2(double d, double d1);

  static double log(double d);
};

}  // namespace bdm

#endif  // MATH_UTIL_H_
