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
#include <typeinfo>
#include "core/shape.h"
#include "core/util/string.h"

namespace bdm {

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

/// Use this cast if you want to downcast an object to a known type with extra
/// safety. The extra safety check will only be performed in Debug mode.
template <typename TTo, typename TFrom>
TTo bdm_static_cast(TFrom from) {  // NOLINT
  assert(dynamic_cast<TTo>(from) &&
         Concat("Could not cast object ", from, " to type ", typeid(TTo).name())
             .c_str());
  return static_cast<TTo>(from);
}

}  // namespace bdm

#endif  // CORE_UTIL_TYPE_H_
