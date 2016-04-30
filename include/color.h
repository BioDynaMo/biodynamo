#ifndef COLOR_H_
#define COLOR_H_

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
  constexpr Color(unsigned value) : value_{value} {}

  Color() : value_{0} {}

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
