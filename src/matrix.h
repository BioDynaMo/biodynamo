#ifndef MATRIX_H_
#define MATRIX_H_

#include <array>

namespace bdm {

using std::array;

class Matrix {
 public:
  /// Add two vectors
  /// @param a the first vector
  /// @param b the second vector
  /// @return a + b
  template <typename T, std::size_t N>
  static std::array<T, N> Add(const array<T, N>& a, const array<T, N>& b) {
    array<T, N> result;
    for (size_t i = 0; i < N; i++) {
      result[i] = a[i] + b[i];
    }
    return result;
  }

  /// Subtract two vectors
  /// @param a
  /// @param b
  /// @return a-b
  template <typename T, std::size_t N>
  static array<T, N> Subtract(const array<T, N>& a, const array<T, N>& b) {
    array<T, N> result;
    for (size_t i = 0; i < N; i++) {
      result[i] = a[i] - b[i];
    }
    return result;
  }

  /// Compute the inner product (also called dot product) of two vectors.
  /// @param a
  /// @param b
  /// @return a.b
  template <typename T, std::size_t N>
  static T Dot(const array<T, N>& a, const array<T, N>& b) {
    T product = 0;
    for (size_t i = 0; i < N; i++) {
      product += a[i] * b[i];
    }
    return product;
  }

  /// Multiplication of (all the elements of) a vector by a scalar value.
  ///
  /// @param  k a scalar
  /// @param  a the vector we want to multiply
  /// @return k * a
  template <std::size_t N>
  static std::array<double, N> ScalarMult(double k,
                                          const std::array<double, N>& a) {
    std::array<double, N> result;
    for (size_t i = 0; i < N; i++) {
      result[i] = a[i] * k;
    }
    return result;
  }
};

}  // namespace bdm

#endif  // MATRIX_H_
