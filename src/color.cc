#include "color.h"

#include <cmath>

namespace bdm {

Color Color::getRandomColor() {
  long r = std::lround(255 * Random::nextDouble());
  long g = std::lround(255 * Random::nextDouble());
  long b = std::lround(255 * Random::nextDouble());

  int color = 0xB3000000;
  color |= r << 16;
  color |= g << 8;
  color |= b;

  return Color(color);
}

Color::Color()
    : value_ { 0 } {
}

bool Color::operator==(const Color& other) const {
  return value_ == other.value_;
}

unsigned Color::getValue() const {
  return value_;
}

}  // namespace bdm
