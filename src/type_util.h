#ifndef TYPE_UTIL_H_
#define TYPE_UTIL_H_

#include <type_traits>
#include <vector>
#include <tuple>

#include "backend.h"
#include "variant.h"

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

// -----------------------------------------------------------------------------
/// Type trait to determine whether type `T` is std::tuple<...>
template <typename T>
struct is_tuple : std::false_type {};

template <typename... T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

// -----------------------------------------------------------------------------
/// Type trait to determine whether type `T` is std::vector<...>
template <typename T>
struct is_vector : std::false_type {};

template <typename T>
struct is_vector<std::vector<T>> : std::true_type {};

// -----------------------------------------------------------------------------
/// Type trait to determine whether type `T` is bdm::Variant<...>
template <typename T>
struct is_Variant : std::false_type {};

template <typename... T>
struct is_Variant<bdm::Variant<T...>> : std::true_type {};

}  // namespace bdm

#endif  // TYPE_UTIL_H_
