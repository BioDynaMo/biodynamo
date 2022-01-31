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

#ifndef CORE_UTIL_LOG_H_
#define CORE_UTIL_LOG_H_

#include <TError.h>

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

#include "core/util/string.h"

namespace bdm {

/// @brief Wrapper class over ROOT logging module
///
class Log {
 public:
  /// @brief      Prints debug message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void Debug(const std::string& location, const Args&... parts) {
    // kPrint has the highest level of verbosity
    if (gErrorIgnoreLevel <= kPrint) {
      std::string message = Concat(parts...);
      // Mimic ROOT logging output
      fprintf(stderr, "Debug in <%s>: %s\n", location.c_str(), message.c_str());
    }
  }

  /// @brief      Prints information message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void Info(const std::string& location, const Args&... parts) {
    std::string message = Concat(parts...);
    // ROOT function
    ::Info(location.c_str(), "%s", message.c_str());
  }

  /// @brief      Prints warning message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void Warning(const std::string& location, const Args&... parts) {
    std::string message = Concat(parts...);
    // ROOT function
    ::Warning(location.c_str(), "%s", message.c_str());
  }

  /// @brief Prints error message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void Error(const std::string& location, const Args&... parts) {
    std::string message = Concat(parts...);
    // ROOT function
    ::Error(location.c_str(), "%s", message.c_str());
  }

  /// @brief Prints break message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void Break(const std::string& location, const Args&... parts) {
    std::string message = Concat(parts...);
    // ROOT function
    ::Break(location.c_str(), "%s", message.c_str());
  }

  /// @brief Prints system error message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void SysError(const std::string& location, const Args&... parts) {
    std::string message = Concat(parts...);
    // ROOT function
    ::SysError(location.c_str(), "%s", message.c_str());
  }

  /// @brief Prints fatal error message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void Fatal(const std::string& location, const Args&... parts) {
    std::string message = Concat(parts...);
    // ROOT function
    ::Error(location.c_str(), "%s", message.c_str());
    // std::runtime_error() will fail the DeathTests
    exit(1);
  }

  template <typename... Args>
  static void Condition(const std::function<bool()>& lambda,
                        const Args&... parts) {
    // kPrint has the highest level of verbosity
    if (lambda()) {
      std::string message = Concat(parts...);
      // Mimic ROOT logging output
      fprintf(stdout, "%s\n", message.c_str());
    }
  }
};
}  // namespace bdm

#endif  // CORE_UTIL_LOG_H_
