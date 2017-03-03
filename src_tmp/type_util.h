#ifndef TYPE_UTIL_H_
#define TYPE_UTIL_H_

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

}  // namespace bdm

#endif  // TYPE_UTIL_H_
