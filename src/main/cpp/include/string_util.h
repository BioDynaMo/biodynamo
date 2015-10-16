#ifndef STRING_UTIL_H_
#define STRING_UTIL_H_

#include <string>
#include <sstream>
#include <array>
#include <list>
#include <memory>
#include <cstdio>

namespace cx3d {

class StringUtil {
 public:
  static constexpr size_t buffer_size_ = 512;

  template<class T>
  static std::string toStr(const std::shared_ptr<T>& s_ptr) {
    return s_ptr.get() != nullptr ? s_ptr->toString() : "null";
  }

  template<class T, size_t N>
  static std::string toStr(const std::array<std::shared_ptr<T>, N>& arr) {
    std::stringstream str;
    str << "{";
    for (auto el : arr) {
      str << p(el) << ", ";
    }
    str << "}";
    return str.str();
  }

  static std::string toStr(int i) {
    return std::to_string(i);
  }

  static std::string toStr(bool b) {
    return b ? "true" : "false";
  }

  static std::string toStr(double d) {
    char buffer[buffer_size_];
    snprintf(buffer, buffer_size_, "%.5f", d);
    return std::string(buffer);
  }

  template<size_t N>
  static std::string toStr(const std::array<double, N>& arr) {
    std::stringstream str;
    str << "{";
    for (auto el : arr) {
      str << toStr(el) << ", ";
    }
    str << "}";
    return str.str();
  }

  template<size_t N, size_t M>
  static std::string toStr(const std::array<std::array<double, N>, M>& arr) {
    std::stringstream str;
    str << "{";
    for (auto el : arr) {
      str << toStr(el) << ", ";
    }
    str << "}";
    return str.str();
  }

  template<class T>
  static std::string toStr(const std::list<std::shared_ptr<T>>& list) {
    std::stringstream str;
    str << "{";
    for (auto el : list) {
      str << toStr(el) << ", ";
    }
    str << "}";
    return str.str();
  }
};

}  // namespace cx3d

#endif  // STRING_UTIL_H_
