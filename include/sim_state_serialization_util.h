#ifndef SIM_STATE_SERIALIZATION_UTIL_H_
#define SIM_STATE_SERIALIZATION_UTIL_H_

#include <list>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <unordered_map>

#include "string_builder.h"
#include "sim_state_serializable.h"

namespace cx3d {

using std::string;
using std::array;

using cx3d::StringBuilder;
using cx3d::SimStateSerializable;

/**
 * Classes that implement that interface serialize their simulation state to
 * json with as little implementation details as possible (e.g. state of locks
 * or which collection implementation has been used)
 */
class SimStateSerializationUtil {
 public:
  static StringBuilder& key(StringBuilder& sb, string key) {
    sb.append("\"").append(key).append("\":");
    return sb;
  }

  static string doubleToString(double d) {
    std::stringstream s;
    s.precision(16);
    s << std::scientific;
    s << d;
    return s.str();
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, string value) {
    return SimStateSerializationUtil::keyValue(sb, key, value, false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, double value) {
    return SimStateSerializationUtil::keyValue(sb, key, SimStateSerializationUtil::doubleToString(value), false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, int value) {
    return SimStateSerializationUtil::keyValue(sb, key, std::to_string(value), false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, std::size_t value) {
    return SimStateSerializationUtil::keyValue(sb, key, std::to_string(value), false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, bool value) {
    return SimStateSerializationUtil::keyValue(sb, key, value ? "true" : "false", false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, string value, bool wrap_with_quotes) {
    SimStateSerializationUtil::key(sb, key);
    if (wrap_with_quotes) {
      sb.append("\"");
    }
    sb.append(value);
    if (wrap_with_quotes) {
      sb.append("\"");
    }
    sb.append(",");
    return sb;
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, const std::shared_ptr<SimStateSerializable>& value) {
    return keyValue(sb, key, value.get());
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, const SimStateSerializable* value) {
    SimStateSerializationUtil::key(sb, key);
    if (value != nullptr) {
      value->simStateToJson(sb);
    } else {
      sb.append("null");
    }
    sb.append(",");
    return sb;
  }

  template<std::size_t N>
  static StringBuilder& keyValue(StringBuilder& sb, string key, const array<double, N>& vector) {
    SimStateSerializationUtil::key(sb, key).append("[");
    for (double value : vector) {
      sb.append(SimStateSerializationUtil::doubleToString(value)).append(",");
    }
    SimStateSerializationUtil::removeLastChar(sb);
    sb.append("],");
    return sb;
  }

  template<class K, class V>
  static StringBuilder& map(StringBuilder& sb, string key, const std::unordered_map<K, V>& map) {
    SimStateSerializationUtil::key(sb, key).append("{");
    for (auto i : map) {
      SimStateSerializationUtil::key(sb, i.first);
      i.second->simStateToJson(sb).append(",");
    }
    if (!map.empty()) {
      SimStateSerializationUtil::removeLastChar(sb);
    }
    sb.append("},");
    return sb;
  }

  static StringBuilder& removeLastChar(StringBuilder& sb) {
    sb.overwriteLastCharOnNextAppend();
    return sb;
  }

  static std::string colorToHexString(uint32_t value) {
    // alpha value was not considered on Java side -> remove here
    uint32_t rgb = value & 0x00ffffff;
    stringstream stream;
    stream << "#";
    stream << std::setw(6) << std::hex << std::setfill('0') << rgb;
    return stream.str();
  }
  template<class T, class U, class V>
  static StringBuilder& mapOfDoubleArray(
      StringBuilder& sb, const string& key,
      const std::unordered_map<std::shared_ptr<T>, std::array<double, 3>, U, V>& map) {
    SimStateSerializationUtil::key(sb, key).append("{");
    for (auto el : map) {
      SimStateSerializationUtil::keyValue(sb, el.first->toString(), el.second);
    }
    if (!map.empty()) {
      removeLastChar(sb);
    }
    sb.append("},");
    return sb;
  }

  template<class T>
  static StringBuilder& unorderedCollection(StringBuilder& sb, const string& key,
                                            const std::list<T>& elements) {
    //simple implementation of unorderedCollection did not work
    //for now forward call to ordered collection (position of an element matters in equality comparisons)
    //if true position invariance in a collection is needed implement a more sophisticated solution
    //e.g. use object instead of array with key = hash(value) -> json string has to be traversed a second time after
    //build
    return orderedCollection(sb, key, elements);
  }

  template<class T>
  static StringBuilder& orderedCollection(StringBuilder& sb, const string& key,
                                          const std::list<T>& elements) {
    SimStateSerializationUtil::key(sb, key).append("[");
    for (auto el : elements) {
      el->simStateToJson(sb);
      sb.append(",");
    }
    if (!elements.empty()) {
      removeLastChar(sb);
    }
    sb.append("],");
    return sb;
  }

  template<class T>
  static StringBuilder& unorderedCollection(StringBuilder& sb, const string& key,
                                            const std::vector<T>& elements) {
    //simple implementation of unorderedCollection did not work
    //for now forward call to ordered collection (position of an element matters in equality comparisons)
    //if true position invariance in a collection is needed implement a more sophisticated solution
    //e.g. use object instead of array with key = hash(value) -> json string has to be traversed a second time after
    //build
    return orderedCollection(sb, key, elements);
  }

  template<class T>
  static StringBuilder& orderedCollection(StringBuilder& sb, const string& key,
                                          const std::vector<T>& elements) {
    SimStateSerializationUtil::key(sb, key).append("[");
    for (auto& el : elements) {
      el->simStateToJson(sb);
      sb.append(",");
    }
    if (!elements.empty()) {
      removeLastChar(sb);
    }
    sb.append("],");
    return sb;
  }
};

}  // namespace cx3d

#endif  // SIM_STATE_SERIALIZATION_UTIL_H_
