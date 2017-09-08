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

///
/// @brief Logging levels enumerator
///
/// Uses existing ROOT values, defined in TError.h header file
///
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

///
/// @brief Wrapper class over ROOT logging module
///
class Logger {
 public:
  ///
  /// @brief Creates new logger object
  ///
  /// Logger will discard messages which belong to logging levels lower that the
  /// \p level argument. Only exception is Logger::Print method that will print
  /// every message, regardless of the \p level.
  ///
  /// @param  location  unique name for logger
  /// @param  level     minimum logging level
  ///
  explicit Logger(const std::string& location,
                  LoggingLevel level = LoggingLevel::kUnset)
      : level_(level), location_(location) {}
  ~Logger() {}

  ///
  /// @brief Print a message to stderr
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Print(const Args&... parts) const {
    auto message = ConstructMessage(parts...);
    // Print to stderr (as ROOT does for every message)
    fprintf(stderr, "%s\n", message->c_str());
  }

  ///
  /// @brief Prints debug message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Debug(const Args&... parts) const {
    if (LoggingLevel::kDebug >= level_) {
      auto message = ConstructMessage(parts...);
      // Emulate ROOT logging message
      fprintf(stderr, "Debug in <%s>: %s\n", location_.c_str(),
              message->c_str());
    }
  }

  ///
  /// @brief Prints information message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Info(const Args&... parts) const {
    if (LoggingLevel::kInfo >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Info(location_.c_str(), "%s", message->c_str());
    }
  }

  ///
  /// @brief Prints warning message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Warning(const Args&... parts) const {
    if (LoggingLevel::kWarning >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Warning(location_.c_str(), "%s", message->c_str());
    }
  }

  ///
  /// @brief Prints error message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Error(const Args&... parts) const {
    if (LoggingLevel::kError >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Error(location_.c_str(), "%s", message->c_str());
    }
  }

  ///
  /// @brief Prints break message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Break(const Args&... parts) const {
    if (LoggingLevel::kBreak >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Break(location_.c_str(), "%s", message->c_str());
    }
  }

  ///
  /// @brief Prints system error message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void SysError(const Args&... parts) const {
    if (LoggingLevel::kSysError >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::SysError(location_.c_str(), "%s", message->c_str());
    }
  }

  ///
  /// @brief Prints fatal error message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Fatal(const Args&... parts) const {
    if (LoggingLevel::kFatal >= level_) {
      auto message = ConstructMessage(parts...);
      // ROOT function
      ::Fatal(location_.c_str(), "%s", message->c_str());
    }
  }

 private:
  ///
  /// @brief Creates the message composed of different objects
  ///
  /// @param[in]  parts objects that compose the entire message
  ///
  /// @returns  A unique string pointer to the message
  ///
  template <typename... Args>
  std::unique_ptr<std::string> ConstructMessage(const Args&... parts) const {
    std::ostringstream message;
    message << open_str_;
    ConcatNextPart(&message, parts...);
    return std::make_unique<std::string>(message.str());
  }

  ///
  /// @brief  Appends the closing string to the message
  ///
  /// @param[in]  ss    the stringstream that holds the message
  ///
  void ConcatNextPart(std::ostringstream* ss) const { *ss << close_str_; }

  ///
  /// @brief Appends the next part of the message
  ///
  /// @param[in]  ss    the stringstream that holds the message
  /// @param[in]  arg   the part to be appended next
  /// @param[in]  parts the rest of the parts, waiting to be appended
  ///
  template <typename T, typename... Args>
  void ConcatNextPart(std::ostringstream* ss, const T& arg,
                      const Args&... parts) const {
    *ss << arg;
    ConcatNextPart(ss, parts...);
  }

  const std::string open_str_ = "";   //!< String to be appended at the end
  const std::string close_str_ = "";  //!< String to be appended at the start

  LoggingLevel level_;    //!< Logging level of this logger
  std::string location_;  //!< Name/Location of this logger
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_LOGGER_H_
