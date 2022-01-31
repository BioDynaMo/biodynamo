// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
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
#include <vector>

namespace bdm {

// https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
inline bool EndsWith(const std::string& str, const std::string& suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline bool StartsWith(const std::string& str, const std::string& prefix) {
  return str.size() >= prefix.size() &&
         str.compare(0, prefix.size(), prefix) == 0;
}

std::vector<std::string> Split(const std::string& s,
                               const std::string& delimiter);

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
