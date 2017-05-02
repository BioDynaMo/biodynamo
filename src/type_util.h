#ifndef TYPE_UTIL_H_
#define TYPE_UTIL_H_

#include <type_traits>
#include "backend.h"

using std::is_same;

namespace bdm {

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

/// Type trait to determine whether a backend is soa
template <typename Backend>
struct is_soa {
  static const bool value =
      is_same<Backend, Soa>::value || is_same<Backend, SoaRef>::value;
};

}  // namespace bdm

#endif  // TYPE_UTIL_H_
