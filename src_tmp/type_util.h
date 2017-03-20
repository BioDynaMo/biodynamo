#ifndef TYPE_UTIL_H_
#define TYPE_UTIL_H_

#include "backend.h"

namespace bdm {

struct EmptyType {};

template <bool condition, typename T, typename U>
struct type_ternary_operator {};

template <typename T, typename U>
struct type_ternary_operator<true, T, U> {
  typedef T type;
};

template <typename T, typename U>
struct type_ternary_operator<false, T, U> {
  typedef U type;
};

// -----------------------------------------------------------------------------

template <typename T>
struct is_std_array {
  static const bool value = false;
};

// exclude type aliases of std::array
// define that type alias VcBackend::SimdArray is not std::array
template <typename T>
struct is_std_array<VcBackend::SimdArray<T>> {
  static const bool value = false;
};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> {
  static const bool value = true;
};

// -----------------------------------------------------------------------------

template <typename T>
struct is_scalar {
  static const bool value = std::is_same<typename T::Backend, ScalarBackend>::value;
};

}  // namespace bdm

#endif  // TYPE_UTIL_H_
