#ifndef TYPE_UTIL_H_
#define TYPE_UTIL_H_

#include <type_traits>
#include "backend.h"

namespace bdm {

using std::is_same;

/// Type trait which defines a ternary operator for types which can be evaluated
/// at compile time
template <bool Condition, typename T, typename U>
struct type_ternary_operator {};  // NOLINT

template <typename T, typename U>
struct type_ternary_operator<true, T, U> {
  typedef T type;  // NOLINT
};

template <typename T, typename U>
struct type_ternary_operator<false, T, U> {
  typedef U type;  // NOLINT
};

/// Type trait to determine whether a backend is soa
template <typename Backend>
struct is_soa {              // NOLINT
  static const bool value =  // NOLINT
      std::is_same<Backend, Soa>::value || std::is_same<Backend, SoaRef>::value;
};

}  // namespace bdm

#endif  // TYPE_UTIL_H_
