#ifndef DEMO_DISTRIBUTION_COMMON_H_
#define DEMO_DISTRIBUTION_COMMON_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace bdm {

typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;
typedef std::chrono::milliseconds duration_ms_t;

struct DistSharedInfo {
  std::vector<std::unique_ptr<zmqpp::message> >* pending_;
  zmqpp::reactor* reactor_;        // Polling handler
  zmqpp::context* ctx_;            // ZMQ context
  std::string identity_;           // Current node identity
  bool verbose_;                   // Print to stdout
  bool zctx_interrupted_ = false;  // ZMQ interrupted by signal
};

template <typename E>
constexpr typename std::underlying_type<E>::type ToUnderlying(E e) {
  return static_cast<typename std::underlying_type<E>::type>(e);
}

// -------- Application level protocol ---------
// ---------------------------------------------

// Communicator identifiers
enum class CommunicatorId : std::uint8_t {
  kUndefined = 0,
  kClient,
  kBroker,
  kLeftNeighbour,
  kRightNeighbour,
  kSomeWorker,

  kMinValue = kClient,
  kMaxValue = kSomeWorker,
  kCount = kSomeWorker
};
const std::string CommunicatorIdStr[] = {"Undefined",      "Client",
                                         "Broker",         "LeftNeighbour",
                                         "RightNeighbour", "SomeWorker"};

inline std::ostream& operator<<(std::ostream& stream,
                                const CommunicatorId& comm_id) {
  stream << CommunicatorIdStr[ToUnderlying(comm_id)];
  return stream;
}

// -------- Majordomo pattern constants --------
// ---------------------------------------------
const std::string MDPC_CLIENT = "MDPC0X";
const std::string MDPW_WORKER = "MDPW0X";

// Heartbeat
const size_t HEARTBEAT_LIVENESS = 3;           //  3-5 is reasonable
const duration_ms_t HEARTBEAT_INTERVAL(2500);  //  msecs
const duration_ms_t HEARTBEAT_EXPIRY = HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS;

// ---------------------------------------------

// Converts a string to its hex representation
inline std::string ToHex(const std::string& in) {
  std::stringstream ss;
  for (size_t i = 0; i < in.length(); i++) {
    ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase
       << (static_cast<int>(in[i]));
  }
  return ss.str();
}

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

}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_COMMON_H__
