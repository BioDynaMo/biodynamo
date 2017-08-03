#ifndef __MDP_COMMON_H__
#define __MDP_COMMON_H__

#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

namespace mdp {

typedef std::chrono::time_point<std::chrono::system_clock> time_point_t;
typedef std::chrono::milliseconds duration_ms_t;

std::ostream& operator<< (std::ostream& out, const zmqpp::message& msg);
std::string toHex(const std::string& in);

const size_t HEARTBEAT_LIVENESS = 3;            //  3-5 is reasonable
const duration_ms_t HEARTBEAT_INTERVAL (2500);  //  msecs
const duration_ms_t HEARTBEAT_EXPIRY = HEARTBEAT_INTERVAL * HEARTBEAT_LIVENESS;

//  This is the version of MDP/Client we implement
const std::string MDPC_CLIENT = "MDPC0X";

//  MDP/Client commands, as strings
const std::string MDPC_REQUEST = "\001";
const std::string MDPC_REPORT = "\002";
const std::string MDPC_NAK = "\003";

static std::string mdpc_commands [] = {
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

static std::string mdpw_commands [] = {
    "INVALID_CMD", "READY", "REQUEST", "REPORT", "HEARTBEAT", "DISCONNECT"
};

}

#endif // __MDP_COMMON_H__
