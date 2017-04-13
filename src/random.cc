#include "random.h"

#include <cmath>
#include <random>

namespace bdm {

void Random::setSeed(int64_t seed) {
  seed_ = (seed ^ 25214903917L) & 281474976710655L;
  next_next_gaussian_ = 0.0;
  have_next_next_gaussian_ = false;
}

int Random::nextInt() { return std::rand(); }

double Random::nextDouble() {
  return static_cast<double>((static_cast<int64_t>(next(26)) << 27) +
                             static_cast<int64_t>(next(27))) *
         1.1102230246251565E-16;
}

double Random::nextGaussian(double mean, double standard_deviation) {
  return mean + standard_deviation * nextGaussian();
}

int Random::next(int var1) {
  int64_t var6 = seed_;

  int64_t var2;
  int64_t var4;
  do {
    var2 = var6;
    var4 = (var2 * 25214903917L + 11L) & 281474976710655L;
  } while (!compareAndSet(&var6, var2, var4));
  seed_ = var6;
  return static_cast<int>(var4 >> (48 - var1));
}

bool Random::compareAndSet(int64_t* current, int64_t expected, int64_t update) {
  if (*current == expected) {
    *current = update;
    return true;
  }
  return false;
}

double Random::nextGaussian() {
  if (have_next_next_gaussian_) {
    have_next_next_gaussian_ = false;
    return next_next_gaussian_;
  } else {
    double v1, v2, s;
    do {
      v1 = 2 * nextDouble() - 1;  // between -1.0 and 1.0
      v2 = 2 * nextDouble() - 1;  // between -1.0 and 1.0
      s = v1 * v1 + v2 * v2;
    } while (s >= 1 || s == 0);
    double multiplier = std::sqrt(-2 * std::log(s) / s);
    next_next_gaussian_ = v2 * multiplier;
    have_next_next_gaussian_ = true;
    return v1 * multiplier;
  }
}

std::array<double, 3> Random::nextNoise(double k) {
  std::array<double, 3> ret;
  ret[0] = -k + 2 * k * nextDouble();
  ret[1] = -k + 2 * k * nextDouble();
  ret[2] = -k + 2 * k * nextDouble();
  return ret;
}

}  // namespace bdm
