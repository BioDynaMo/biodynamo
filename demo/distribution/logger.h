#ifndef DEMO_DISTRIBUTION_LOGGER_H_
#define DEMO_DISTRIBUTION_LOGGER_H_

#include <TError.h>
#include <cstdio>

#include <iostream>
#include <sstream>
#include <string>
#include <tuple>

#include "message.h"

namespace bdm {

// ROOT logging levels
enum class LoggingLevel : Int_t {
  kUnset = ::kUnset,
  kDebug = ::kPrint,
  kInfo = ::kInfo,
  kWarning = ::kWarning,
  kError = ::kError,
  kBreak = ::kBreak,
  kSysError = ::kSysError,
  kFatal = ::kFatal
};

// Wrapper class over ROOT logging module
class Logger {
 public:
  explicit Logger(const std::string& location,
                  LoggingLevel level = LoggingLevel::kUnset)
      : level_(level), location_(location) {}
  ~Logger() {}

  template <typename... Args>
  inline void Print(const Args&... parts) const {
    auto message = ConstructMessage(parts...);
    // Print to stderr (as ROOT does for every message)
    fprintf(stderr, "%s\n", message->c_str());
  }

  template <typename... Args>
  inline void Debug(const Args&... parts) const {
    if (LoggingLevel::kDebug >= level_) {
      auto message = ConstructMessage(parts...);
      // Emulate ROOT logging message
      fprintf(stderr, "Debug in <%s>: %s\n", location_.c_str(),
              message->c_str());
    }
  }

  template <typename... Args>
  inline void Info(const Args&... parts) const {
    if (LoggingLevel::kInfo >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Info(location_.c_str(), "%s", message->c_str());
    }
  }

  template <typename... Args>
  inline void Warning(const Args&... parts) const {
    if (LoggingLevel::kWarning >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Warning(location_.c_str(), "%s", message->c_str());
    }
  }

  template <typename... Args>
  inline void Error(const Args&... parts) const {
    if (LoggingLevel::kError >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Error(location_.c_str(), "%s", message->c_str());
    }
  }

  template <typename... Args>
  inline void Break(const Args&... parts) const {
    if (LoggingLevel::kBreak >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Break(location_.c_str(), "%s", message->c_str());
    }
  }

  template <typename... Args>
  inline void SysError(const Args&... parts) const {
    if (LoggingLevel::kSysError >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::SysError(location_.c_str(), "%s", message->c_str());
    }
  }

  template <typename... Args>
  inline void Fatal(const Args&... parts) const {
    if (LoggingLevel::kFatal >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Fatal(location_.c_str(), "%s", message->c_str());
    }
  }

 private:
  template <typename... Args>
  std::unique_ptr<std::string> ConstructMessage(const Args&... parts) const {
    std::ostringstream message;
    message << open_str_;
    ConcatNextPart(&message, parts...);
    return std::make_unique<std::string>(message.str());
  }

  // Base case
  void ConcatNextPart(std::ostringstream* ss) const { *ss << close_str_; }

  // General case
  template <typename T, typename... Args>
  void ConcatNextPart(std::ostringstream* ss, const T& arg,
                      const Args&... parts) const {
    *ss << arg;
    ConcatNextPart(ss, parts...);
  }

  const std::string open_str_ = "";
  const std::string close_str_ = "";

  LoggingLevel level_;
  std::string location_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_LOGGER_H_
