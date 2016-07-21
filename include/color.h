#ifndef COLOR_H_
#define COLOR_H_

#include "random.h"

namespace bdm {

/**
 * C++ representation of java.awt.Color
 * alpha component in bits 24-31
 * red component in bits   16-23
 * green component in bits  8-15
 * blue component in bits   0- 7
 */
class Color {
 public:
  static Color getRandomColor();

  constexpr Color(unsigned value)
      : value_ { value } {
  }

  Color();

  bool operator==(const Color& other) const;

  unsigned getValue() const;

 private:
  unsigned value_;
};

}  // namespace bdm

#endif  // COLOR_H_
