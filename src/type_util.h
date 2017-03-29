#ifndef TYPE_UTIL_H_
#define TYPE_UTIL_H_

#include <type_traits>
#include "backend.h"

using std::is_same;

namespace bdm {

struct EmptyType {};

/// Type trait which defines a ternary operator for types which can be evaluated
/// at compile time
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

/// Type trait to determine whether a given type is a std::array
/// Type alias `VcVectorBackend::SimdArray` is excluded
template <typename T>
struct is_std_array {
  static const bool value = false;
};

/// Exclude type alias of std::array
/// Define that type alias VcVectorBackend::SimdArray is not std::array
template <typename T>
struct is_std_array<VcVectorBackend::SimdArray<T>> {
  static const bool value = false;
};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> {
  static const bool value = true;
};

/// Type trait to determine whether a simulation object is using a scalar
/// backend
template <typename T>
struct is_scalar {
  static const bool value = is_same<typename T::Backend, ScalarBackend>::value;
};

template <typename Backend>
struct is_soa {
  static const bool value = is_same<Backend, VcSoaBackend>::value ||
                            is_same<Backend, VcSoaRefBackend>::value;
};

}  // namespace bdm

#endif  // TYPE_UTIL_H_
