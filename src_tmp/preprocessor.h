#ifndef PREPROCESSOR_H_
#define PREPROCESSOR_H_

#include "cpp_magic.h"

/// loops over variadic macro arguments and calls the specified operation
/// processes one argument in each iteration
/// e.g. LOOP(OP, a, b) will lead to:
/// OP(a)
/// OP(b)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP(operation, first, ...)                          \
  operation(first)                                           \
  IF(HAS_ARGS(__VA_ARGS__))(                                 \
    DEFER2(_LOOP)()(operation, __VA_ARGS__))
#define _LOOP() LOOP
// clang-format on

/// loops over variadic macro arguments and calls the specified operation
/// removes the first two parameters in each iteration, but adds the first one
/// again for the next call
/// e.g. LOOP_3_1(OP, a, b, c) will lead to:
/// OP(a, b)
/// OP(a, c)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP_2_1(operation, first, second, ...)           \
  operation(first, second)                                \
  IF(HAS_ARGS(__VA_ARGS__))(                              \
    DEFER2(_LOOP_2_1)()(operation, first, __VA_ARGS__))
#define _LOOP_2_1() LOOP_2_1
// clang-format on

/// loops over variadic macro arguments and calls the specified operation
/// removes the first three parameters in each iteration, but adds the first one
/// again for the next call
/// e.g. LOOP_3_1(OP, a, b, c, d, e) will lead to:
/// OP(a, b, c)
/// OP(a, d, e)
/// For a more detailed explanation see `MAP` macro in `third_party/cpp_magic.h`
// clang-format off
#define LOOP_3_1(operation, first, second, third, ...)       \
  operation(first, second, third)                            \
  IF(HAS_ARGS(__VA_ARGS__))(                                 \
    DEFER2(_LOOP_3_1)()(operation, first, __VA_ARGS__))
#define _LOOP_3_1() LOOP_3_1
// clang-format on

#endif  // PREPROCESSOR_H_
