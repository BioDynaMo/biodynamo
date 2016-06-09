#ifndef RANDOM_H_
#define RANDOM_H_

#include <array>
#include <cstdio>

namespace bdm {

/**
 * C++ implementation of the Java default random number generator (java.util.Random)
 */
class Random {
 public:
  Random() = delete;

  static void setSeed(long seed);

  static int nextInt();

  static double nextDouble();

  static double nextGaussian(double mean, double standard_deviation);

  static std::array<double, 3> nextNoise(double k);

 private:
  static long seed_;
  static double next_next_gaussian_;
  static bool have_next_next_gaussian_;

  static int next(int i);

  static double nextGaussian();

  static bool compareAndSet(long& current, long expected, long update);
};

}  // namespace bdm

#endif  //RANDOM_H_
