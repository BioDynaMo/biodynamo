#ifndef __MDP_COMMON_H__
#define __MDP_COMMON_H__

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include <Rtypes.h>

namespace bdm {

typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;
typedef std::chrono::milliseconds duration_ms_t;

std::ostream& operator<<(std::ostream& out, const zmqpp::message& msg);
std::string toHex(const std::string& in);

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
  kBroker,
  kLeftNeighbour,
  kRightNeighbour,
  kCount,

  kMinValue = kBroker,
  kMaxValue = kRightNeighbour,
};

// Distributed API commands
enum class AppProtocolCommand : std::uint8_t {
  kDebug = 0,

  kCount,
  kMinValue = kDebug,
  kMaxValue = kDebug,
};

class AppMessageHeader {
  AppProtocolCommand cmd;
  CommunicatorId sender;

  // ClassDef(AppMessageHeader, 1);
};

// -------- Majordomo pattern constants --------
// ---------------------------------------------

//  This is the version of MDP/Client we implement
const std::string MDPC_CLIENT = "MDPC0X";

//  MDP/Client commands, as strings
const std::string MDPC_REQUEST = "\001";
const std::string MDPC_REPORT = "\002";
const std::string MDPC_NAK = "\003";

static std::string mdpc_commands[] = {
    "INVALID_CMD", "REQUEST", "REPORT", "NAK",
};

//  This is the version of MDP/Worker we implement
const std::string MDPW_WORKER = "MDPW0X";

//  MDP/Worker commands, as strings
const std::string MDPW_READY = "\001";
const std::string MDPW_REQUEST = "\002";
const std::string MDPW_REPORT = "\003";
const std::string MDPW_HEARTBEAT = "\004";
const std::string MDPW_DISCONNECT = "\005";

static std::string mdpw_commands[] = {"INVALID_CMD", "READY",     "REQUEST",
                                      "REPORT",      "HEARTBEAT", "DISCONNECT"};

// Heartbeat
const size_t HEARTBEAT_LIVENESS = 3;           //  3-5 is reasonable
const duration_ms_t HEARTBEAT_INTERVAL(2500);  //  msecs
const duration_ms_t HEARTBEAT_EXPIRY = HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS;

// ---------------------------------------------
}  // namespace bdm

#endif  // __MDP_COMMON_H__
