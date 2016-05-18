#ifndef STRING_UTIL_H_
#define STRING_UTIL_H_

#include <string>
#include <sstream>
#include <array>
#include <list>
#include <memory>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <initializer_list>

/**
 * Macro definitions used to output debugging statements
 */
#define logCall(...) \
  StringUtil::logMethodCall(__FUNCTION__, this->toString(), __VA_ARGS__);

#define logCallParameterless() \
    StringUtil::logMethodCall(__FUNCTION__, this->toString());

#define logReturn(RETURN_VALUE) \
    StringUtil::logMethodReturn(__FUNCTION__, this->toString(), RETURN_VALUE);

#define logReturnVoid() \
    StringUtil::logMethodReturn(__FUNCTION__, this->toString());

#define logReturnStatic(RETURN_VALUE) \
    StringUtil::logMethodReturnStatic(__FUNCTION__, RETURN_VALUE);

#define logConstr(CLASS_NAME, ...) \
    StringUtil::logMethodCall(CLASS_NAME" created", this->toString(), __VA_ARGS__);

#define logConstrParameterless(CLASS_NAME) \
    StringUtil::logMethodCall(CLASS_NAME" created", this->toString());

#define logConstrFromStatic(CLASS_NAME, OBJECT, ...) \
    StringUtil::logMethodCall(CLASS_NAME" created", OBJECT->toString(), __VA_ARGS__);

namespace cx3d {

class StringUtil {
 public:
  static constexpr size_t buffer_size_ = 512;

  template<typename ... Args>
  static void logMethodCall(const std::string& method_name, const std::string& internal_state, Args ... args) {
    // unsigned line = getLineNumber();
    std::ostringstream oss;
    oss << "DBG " << (method_name != "equalTo" ? method_name : "equals") << " args: {";
    StringUtil::processParameters(oss, args...);
    oss << "} innerState: " + internal_state;
    std::cout << oss.str() << std::endl;
  }

  static void logMethodCall(const std::string& method_name, const std::string& internal_state) {
    // unsigned line = getLineNumber();
    std::ostringstream oss;
    oss << "DBG " << (method_name != "equalTo" ? method_name : "equals") << " args: {";
    oss << "} innerState: " + internal_state;
    std::cout << oss.str() << std::endl;
  }

  template<class T>
  static void logMethodReturn(const std::string& method_name, const std::string& internal_state, T t) {
    // unsigned line = getLineNumber();
    std::ostringstream oss;
    oss << "DBG " << (method_name != "equalTo" ? method_name : "equals") << " return " << toStr(t)
        << " innerState: " + internal_state;
    std::cout << oss.str() << std::endl;
  }

  static void logMethodReturn(const std::string& method_name, const std::string& internal_state) {
    // unsigned line = getLineNumber();
    std::ostringstream oss;
    oss << "DBG " << (method_name != "equalTo" ? method_name : "equals") << " return  innerState: " << internal_state;
    std::cout << oss.str() << std::endl;
  }

  template<class T>
  static void logMethodReturnStatic(const std::string& method_name, T t) {
    // unsigned line = getLineNumber();
    std::ostringstream oss;
    oss << "DBG " << (method_name != "equalTo" ? method_name : "equals") << " return " << toStr(t);
    std::cout << oss.str() << std::endl;
  }

  static int getLineNumber() {
    static int line_number = 0;
    return ++line_number;
  }

  template<typename T>
  static void processParameters(std::ostream& o, T t) {
    o << toStr(t) << ", ";
  }

  template<typename T, typename ... Args>
  static void processParameters(std::ostream& o, T t, Args ... args) {
    StringUtil::processParameters(o, t);
    StringUtil::processParameters(o, args...);
  }

  template<class T>
  static std::string toStr(const T* ptr) {
    return ptr != nullptr ? ptr->toString() : "null";
  }

  template<class T>
  static std::string toStr(const std::shared_ptr<T>& s_ptr) {
    return s_ptr.get() != nullptr ? s_ptr->toString() : "null";
  }

  static std::string toStr(const std::string& str) {
    return str;
  }

  template<class T, size_t N>
  static std::string toStr(const std::array<T, N>& arr) {
    std::stringstream str;
    str << "{";
    for (auto el : arr) {
      str << toStr(el) << ", ";
    }
    str << "}";
    return str.str();
  }

  static std::string toStr(int i) {
    return std::to_string(i);
  }

  static std::string toStr(std::size_t i) {
    return std::to_string(i);
  }

  static std::string toStr(bool b) {
    return b ? "true" : "false";
  }

  static std::string toStr(double d) {
    static bool set_locale = false;
    if (!set_locale) {
      std::locale::global(std::locale::classic());
      set_locale = true;
    }
    union {
      long long i;
      double d;
    } value;

    value.d = d;
    std::stringstream stream;
    stream << std::setw(16) << std::hex << std::setfill('0') << value.i;
    return stream.str();
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

  template<size_t N>
  static std::string toStr(const std::array<int, N>& arr) {
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
  static std::string toStr(const std::list<T>& list) {
    std::stringstream str;
    str << "{";
    for (auto el : list) {
      str << toStr(el) << ", ";
    }
    str << "}";
    return str.str();
  }

  template<class K, class V, class E, class H>
  static std::string toStr(const std::unordered_map<K, V, E, H>& map) {
    std::list<std::string> list;
    for (auto& i : map) {
      std::stringstream strEntry;
      strEntry << "(" << toStr(i.first) << " -> " << toStr(i.second.get()) << "), ";
      list.push_back(strEntry.str());
    }
    list.sort();
    std::stringstream str;
    str << "{";
    for(auto entry : list){
      str << entry;
    }
    str << "}";
    return str.str();
  }
};

}  // namespace cx3d

#endif  // STRING_UTIL_H_
