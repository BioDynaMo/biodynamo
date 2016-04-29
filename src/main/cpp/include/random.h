#ifndef RANDOM_H_
#define RANDOM_H_

#include "java_util.h"

namespace cx3d {

// todo change to C++11 random implementation
class Random {
 public:
  Random() = delete;

  static void setSeed(double seed);

  static double nextDouble();

  static double nextGaussian(double mean, double standard_deviation);

 private:
  static double nextNextGaussian;
  static bool haveNextNextGaussian;

  static double nextGaussian();
};

}  // namespace cx3d

#endif  //RANDOM_H_
