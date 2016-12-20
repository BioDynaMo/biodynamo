#ifndef RANDOM_H_
#define RANDOM_H_

#include <array>
#include <cstdio>

namespace bdm {

/// C++ implementation of the Java default random number generator
/// (java.util.Random)
class Random {
 public:
  Random(){};

  void setSeed(long seed);

  int nextInt();

  double nextDouble();

  double nextGaussian(double mean, double standard_deviation);

  std::array<double, 3> nextNoise(double k);

  template <typename Backend>
  std::array<typename Backend::real_v, 3> NextNoise(
      const typename Backend::real_v& k) {
    std::array<typename Backend::real_v, 3> ret;
    for (size_t i = 0; i < Backend::kVecLen; i++) {
      // todo not most cache friendly way
      ret[0][i] = -k[i] + 2 * k[i] * nextDouble();
      ret[1][i] = -k[i] + 2 * k[i] * nextDouble();
      ret[2][i] = -k[i] + 2 * k[i] * nextDouble();
    }
    return ret;
  }

 private:
  long seed_ = 0;
  double next_next_gaussian_ = 0.0;
  bool have_next_next_gaussian_ = false;

  int next(int i);

  double nextGaussian();

  bool compareAndSet(long& current, long expected, long update);
};

}  // namespace bdm

#endif  // RANDOM_H_
