#ifndef RANDOM_H_
#define RANDOM_H_

#include <array>
#include <cstdio>

namespace cx3d {

// todo change to C++11 random implementation
class Random {
 public:
  Random() = delete;

  static void setSeed(double seed);

  static int nextInt();

  static double nextDouble();

  static double nextGaussian(double mean, double standard_deviation);

  static std::array<double, 3> nextNoise(double k);

 private:
  static double nextNextGaussian;
  static bool haveNextNextGaussian;

  static double nextGaussian();
};

}  // namespace cx3d

#endif  //RANDOM_H_
