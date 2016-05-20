#include "random.h"

#include <cmath>
#include <random>

namespace cx3d {

double Random::nextNextGaussian = 0.0;
bool Random::haveNextNextGaussian = false;

void Random::setSeed(double seed) {
  std::srand(seed);
}

int Random::nextInt() {
  return std::rand();
}

double Random::nextDouble() {
  return static_cast<double>(std::rand()) / RAND_MAX;
}

double Random::nextGaussian(double mean, double standard_deviation) {
  return mean + standard_deviation * nextGaussian();
}

double Random::nextGaussian() {
  if (haveNextNextGaussian) {
    haveNextNextGaussian = false;
    return nextNextGaussian;
  } else {
    double v1, v2, s;
    do {
      v1 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
      v2 = 2 * nextDouble() - 1;   // between -1.0 and 1.0
      s = v1 * v1 + v2 * v2;
    } while (s >= 1 || s == 0);
    double multiplier = std::sqrt(-2 * std::log(s) / s);
    nextNextGaussian = v2 * multiplier;
    haveNextNextGaussian = true;
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

}  // namespace cx3d
