#ifndef RANDOM_H_
#define RANDOM_H_

#include <array>
#include <cstdio>

#include <TRandom3.h>

namespace bdm {

/// C++ implementation of the Java default random number generator
/// (java.util.Random)
class Random {
 public:
  Random() {}

  void SetSeed(int64_t seed);

  int NextInt();

  double NextDouble();

  double NextGaussian(double mean, double standard_deviation);

  std::array<double, 3> NextNoise(double k);

  template <typename Backend>
  std::array<typename Backend::real_v, 3> NextNoise(
      const typename Backend::real_v& k) {
    std::array<typename Backend::real_v, 3> ret;
    for (size_t i = 0; i < Backend::kVecLen; i++) {
      // todo not most cache friendly way
      ret[0][i] = -k[i] + 2 * k[i] * NextDouble();
      ret[1][i] = -k[i] + 2 * k[i] * NextDouble();
      ret[2][i] = -k[i] + 2 * k[i] * NextDouble();
    }
    return ret;
  }

 private:
  int64_t seed_ = 0;
  double next_next_gaussian_ = 0.0;
  bool have_next_next_gaussian_ = false;

  int Next(int i);

  double NextGaussian();

  bool CompareAndSet(int64_t* current, int64_t expected, int64_t update);
};

extern thread_local Random gRandom;
extern thread_local TRandom3 gTRandom;

}  // namespace bdm

#endif  // RANDOM_H_
