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

#ifndef CORE_UTIL_STRING_H_
#define CORE_UTIL_STRING_H_

#include <sstream>
#include <string>

namespace bdm {

namespace detail {

/// @brief  Appends the closing string to the message
///
/// @param[in]  ss    the stringstream that holds the message
///
inline void ConcatNextPart(std::ostringstream* ss) {}

/// @brief Appends the next part of the message
///
/// @param[in]  ss    the stringstream that holds the message
/// @param[in]  arg   the part to be appended next
/// @param[in]  parts the rest of the parts, waiting to be appended
///
template <typename T, typename... Args>
inline void ConcatNextPart(std::ostringstream* ss, const T& arg,
                           const Args&... parts) {
  *ss << arg;
  ConcatNextPart(ss, parts...);
}

}  // namespace detail

/// @brief Concatenates all arguments into a string.
/// Equivalent to streaming all arguments into a stringstream and calling
/// `stream.str()`
///
/// @param[in]  parts objects that compose the entire message
///
/// @returns  A unique string pointer to the message
///
template <typename... Args>
inline std::string Concat(const Args&... parts) {
  std::ostringstream message;
  detail::ConcatNextPart(&message, parts...);
  return message.str();
}

}  // namespace bdm

#endif  // CORE_UTIL_STRING_H_
