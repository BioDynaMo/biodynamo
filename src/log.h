#ifndef LOG_H_
#define LOG_H_

#include <TError.h>
#include <cstdio>

#include <iostream>
#include <sstream>
#include <string>

namespace bdm {

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

/// @brief Wrapper class over ROOT logging module
///
class Log {
 public:
  /// Logger will discard messages which belong to logging levels lower that the
  /// \p level argument. Only exception is Logger::Print method that will print
  /// every message, regardless of the \p level.
  static void SetLoggingLevel(LoggingLevel level) { gErrorIgnoreLevel = (Int_t) level; }

  /// @brief Prints debug message
  ///
  /// @param[in]  parts   objects that compose the entire message
  ///
  template <typename... Args>
  inline void Debug(const std::string& location, const Args&... parts) {
    if (gErrorIgnoreLevel <= (Int_t) LoggingLevel::kDebug) {
      std::string message = ConstructMessage(parts...);
      // Emulate ROOT logging message
      fprintf(stderr, "Debug in <%s>: %s\n", location.c_str(),
              message.c_str());
    }
  }

  /// @brief Prints information message
  ///
  /// @param[in]  location  The location of the message
  /// @param[in]  parts     objects that compose the entire message
  ///
  template <typename... Args>
  static void Info(const std::string& location, const Args&... parts) {
    std::string message = ConstructMessage(parts...);
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
    std::string message = ConstructMessage(parts...);
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
    std::string message = ConstructMessage(parts...);
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
    std::string message = ConstructMessage(parts...);
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
    std::string message = ConstructMessage(parts...);
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
    std::string message = ConstructMessage(parts...);
    // ROOT function
    ::Fatal(location.c_str(), "%s", message.c_str());
  }

 private:
  /// @brief Creates the message composed of different objects
  ///
  /// @param[in]  parts objects that compose the entire message
  ///
  /// @returns  A unique string pointer to the message
  ///
  template <typename... Args>
  static std::string ConstructMessage(const Args&... parts) {
    std::ostringstream message;
    ConcatNextPart(&message, parts...);
    return message.str();
  }

  /// @brief  Appends the closing string to the message
  ///
  /// @param[in]  ss    the stringstream that holds the message
  ///
  static void ConcatNextPart(std::ostringstream* ss) {}

  /// @brief Appends the next part of the message
  ///
  /// @param[in]  ss    the stringstream that holds the message
  /// @param[in]  arg   the part to be appended next
  /// @param[in]  parts the rest of the parts, waiting to be appended
  ///
  template <typename T, typename... Args>
  static void ConcatNextPart(std::ostringstream* ss, const T& arg,
                      const Args&... parts) {
    *ss << arg;
    ConcatNextPart(ss, parts...);
  }
};
}  // namespace bdm

#endif  // LOG_H_
