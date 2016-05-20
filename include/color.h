#ifndef COLOR_H_
#define COLOR_H_

#include <cmath>
#include "random.h"

namespace cx3d {

/**
 * C++ representation of java.awt.Color
 * alpha component in bits 24-31
 * red component in bits   16-23
 * green component in bits  8-15
 * blue component in bits   0- 7
 */
class Color {
 public:
  static Color getRandomColor() {
    long r = std::lround(255 * Random::nextDouble());
    long g = std::lround(255 * Random::nextDouble());
    long b = std::lround(255 * Random::nextDouble());

    int color = 0xB3000000;
    color |= r << 16;
    color |= g << 8;
    color |= b;

    return Color(color);
  }

  constexpr Color(unsigned value)
      : value_ { value } {
  }

  Color()
      : value_ { 0 } {
  }

  bool operator==(const Color& other) const {
    return value_ == other.value_;
  }

  unsigned getValue() const {
    return value_;
  }

 private:
  unsigned value_;
};

}  // namespace cx3d

#endif  // COLOR_H_
