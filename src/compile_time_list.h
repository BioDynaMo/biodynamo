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
#ifndef COMPILE_TIME_LIST_H_
#define COMPILE_TIME_LIST_H_

#include "variant.h"

namespace bdm {

// Inspiration taken from:
// https://stackoverflow.com/questions/4691657/is-it-possible-to-store-a-template-parameter-pack-without-expanding-it

/// This solution is a workaround to pass around template parameter packs
/// without expanding them. This is necesary since creating a typedef or alias
/// fails. The following code example shows how to extract the parameter pack:
///
///     // Example: Create a std::tuple from a given CTList
///     // Type definition
///     template <typename... Types>
///     struct ConvertToTuple {};
///
///     // Template specialization with automatic type deduction gives back
///     // the original parameter pack
///     template<typename... Types>
///     struct ConvertToTuple< CTList<Types...> > {
///        typedef std::tuple<Types...> type;
///     }
///
///     // code that wants to pass on a parameter pack to a class
///     MyClass<CTList<int, float>> class;
///
///     // client code that receives a CTList type and wants to create
///     // a tuple can do so by:
///     typename ConvertToTuple<TCTList>::type tuple;
template <typename... Types>
struct CTList {
  using Variant_t = Variant<Types...>;
};

}  // namespace bdm

#endif  // COMPILE_TIME_LIST_H_
