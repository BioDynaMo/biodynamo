#ifndef DEMO_DISTRIBUTION_MESSAGE_H_
#define DEMO_DISTRIBUTION_MESSAGE_H_

#include <zmqpp/zmqpp.hpp>

#include <string>

namespace bdm {

///
/// @brief Converts a string to its hex representation
///
/// @param[in]  in  the string obj
///
/// @return Hex representation of the \p in string
///
inline std::string ToHex(const std::string& in) {
  std::stringstream ss;
  for (size_t i = 0; i < in.length(); i++) {
    ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
       << (static_cast<int>(in[i]));
  }
  return ss.str();
}

///
/// @brief Overloaded stream operator for printing zmq messages
///
inline std::ostream& operator<<(std::ostream& out, const zmqpp::message& msg) {
  size_t frames = msg.parts();
  if (!frames) {
    out << std::endl << "*EMPTY_MESSAGE*" << std::endl;
    return out;
  }

  std::ostringstream ss;

  std::string part;
  for (size_t i = 0; i < msg.parts(); i++) {
    msg.get(part, i);

    // probe message
    bool is_binary = false;
    for (size_t j = 0; j < part.size(); j++) {
      if (part[j] < 9 || part[j] > 127) {
        is_binary = true;
        break;
      }
    }

    size_t max_size = is_binary ? 35 : 70;
    std::string ellipses = "";

    if (part.size() > max_size) {
      part.resize(max_size);
      ellipses = "...";
    }
    ss << "[" << std::setfill('0') << std::setw(3) << part.size() << "] ";

    if (is_binary) {
      ss << "0x" << ToHex(part);
    } else {
      ss << part;
    }
    ss << ellipses << std::endl;
  }

  out << std::endl << ss.str();
  return out;
}
};  // namespace bdm

#endif  // DEMO_DISTRIBUTION_MESSAGE_H_
