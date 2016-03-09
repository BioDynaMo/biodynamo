#ifndef SIM_STATE_SERIALIZATION_UTIL_H_
#define SIM_STATE_SERIALIZATION_UTIL_H_

#include <array>
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
    return SimStateSerializationUtil::keyValue(
        sb, key, SimStateSerializationUtil::doubleToString(value), false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, int value) {
    return SimStateSerializationUtil::keyValue(sb, key, std::to_string(value),
                                               false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, std::size_t value) {
      return SimStateSerializationUtil::keyValue(sb, key, std::to_string(value), false);
    }

  static StringBuilder& keyValue(StringBuilder& sb, string key, bool value) {
    return SimStateSerializationUtil::keyValue(sb, key, value ? "true" : "false", false);
  }

  static StringBuilder& keyValue(StringBuilder& sb, string key, string value,
                                 bool wrap_with_quotes) {
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

  static StringBuilder& keyValue(StringBuilder& sb, string key,
                                 const std::shared_ptr<SimStateSerializable>& value) {
    SimStateSerializationUtil::key(sb, key);
    if(value.get() != nullptr) {
      value->simStateToJson(sb);
    } else {
      sb.append("null");
    }
    sb.append(",");
    return sb;
  }

  template<std::size_t N>
  static StringBuilder& keyValue(StringBuilder& sb, string key,
                                 const array<double, N>& vector) {
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

  static std::string colorToHexString(uint32_t value){
    // alpha value was not considered on Java side -> remove here
    uint32_t rgb = value & 0x00ffffff;
    stringstream stream;
    stream << "#";
    stream << std::setw(6) << std::hex << std::setfill('0') << rgb;
    return stream.str();
  }
};

}  // namespace cx3d

#endif  // SIM_STATE_SERIALIZATION_UTIL_H_
