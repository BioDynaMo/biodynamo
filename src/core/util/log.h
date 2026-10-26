// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <string>

#include "core/util/string.h"

namespace bdm {

/// Log levels. The numbers match ROOT's (kPrint, kInfo, ...) so existing
/// verbosity settings keep their meaning; messages below the current level are dropped.
enum LogLevel : int {
  kLogPrint = 0,
  kLogInfo = 1000,
  kLogWarning = 2000,
  kLogError = 3000,
  kLogBreak = 4000,
  kLogSysError = 5000,
  kLogFatal = 6000
};

/// @brief Logging without ROOT. Output keeps ROOT's format: "<Level> in <location>: message"
///
class Log {
 public:
  /// Messages with a level below this are not printed (ROOT: gErrorIgnoreLevel).
  static int GetLevel() { return LevelStorage().load(std::memory_order_relaxed); }
  static void SetLevel(int level) { LevelStorage().store(level, std::memory_order_relaxed); }

  template <typename... Args>
  static void Debug(const std::string& location, const Args&... parts) {
    // kLogPrint has the highest level of verbosity
    if (GetLevel() <= kLogPrint) {
      Print("Debug", location, Concat(parts...));
    }
  }

  template <typename... Args>
  static void Info(const std::string& location, const Args&... parts) {
    Emit(kLogInfo, "Info", location, Concat(parts...));
  }

  template <typename... Args>
  static void Warning(const std::string& location, const Args&... parts) {
    Emit(kLogWarning, "Warning", location, Concat(parts...));
  }

  template <typename... Args>
  static void Error(const std::string& location, const Args&... parts) {
    Emit(kLogError, "Error", location, Concat(parts...));
  }

  template <typename... Args>
  static void Break(const std::string& location, const Args&... parts) {
    Emit(kLogBreak, "Break", location, Concat(parts...));
  }

  template <typename... Args>
  static void SysError(const std::string& location, const Args&... parts) {
    const int err = errno;
    Emit(kLogSysError, "SysError", location,
         Concat(parts...) + " (" + std::strerror(err) + ")");
  }

  template <typename... Args>
  static void Fatal(const std::string& location, const Args&... parts) {
    Emit(kLogError, "Error", location, Concat(parts...));
    // std::runtime_error() will fail the DeathTests
    exit(1);
  }

  template <typename... Args>
  static void Condition(const std::function<bool()>& lambda,
                        const Args&... parts) {
    if (lambda()) {
      std::string message = Concat(parts...);
      fprintf(stdout, "%s\n", message.c_str());
    }
  }

 private:
  static std::atomic<int>& LevelStorage() {
    static std::atomic<int> level{kLogPrint};
    return level;
  }
  static void Emit(int level, const char* tag, const std::string& location,
                   const std::string& message) {
    if (level >= GetLevel()) {
      Print(tag, location, message);
    }
  }
  static void Print(const char* tag, const std::string& location,
                    const std::string& message) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    fflush(stdout);
    fprintf(stderr, "%s in <%s>: %s\n", tag, location.c_str(), message.c_str());
    fflush(stderr);
  }
};
}  // namespace bdm

#endif  // CORE_UTIL_LOG_H_
