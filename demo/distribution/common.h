#ifndef DEMO_DISTRIBUTION_COMMON_H_
#define DEMO_DISTRIBUTION_COMMON_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "logger.h"

namespace bdm {

typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;
typedef std::chrono::milliseconds duration_ms_t;

template <typename E>
constexpr typename std::underlying_type<E>::type ToUnderlying(E e) {
  return static_cast<typename std::underlying_type<E>::type>(e);
}

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

typedef std::pair<std::unique_ptr<zmqpp::message>, CommunicatorId>
    MessageMiddlewareHeaderPair;

struct DistSharedInfo {
  // Messages destined to application; waiting for processing
  std::queue<MessageMiddlewareHeaderPair> mq_app_deliver_;

  zmqpp::reactor reactor_;      // Polling handler
  zmqpp::context* ctx_;         // ZMQ context
  std::string identity_;        // Current node identity
  LoggingLevel logging_level_;  // What logger prints
};

// -------- Majordomo pattern constants --------
// ---------------------------------------------

// Heartbeat
const size_t HEARTBEAT_LIVENESS = 3;           //  3-5 is reasonable
const duration_ms_t HEARTBEAT_INTERVAL(2500);  //  msecs
const duration_ms_t HEARTBEAT_EXPIRY = HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS;

// ---------------------------------------------

}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_COMMON_H__
