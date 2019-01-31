// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_UTIL_TYPE_H_
#define CORE_UTIL_TYPE_H_

#include <type_traits>
#include "core/shape.h"

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

/// Type trait that converts `T*`, `T&`, `T&&`, `T*&` to `T`
template <typename T>
using raw_type = std::remove_pointer_t<std::decay_t<T>>;  // NOLINT

}  // namespace bdm

#endif  // CORE_UTIL_TYPE_H_
