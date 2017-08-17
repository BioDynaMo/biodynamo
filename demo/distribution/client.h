#ifndef __MDP_CLIENT_H__
#define __MDP_CLIENT_H__

#include <chrono>
#include <string>

#include "common.h"

namespace bdm {

class Client {
 public:
  Client(zmqpp::context *ctx, const std::string &broker, bool verbose);
  ~Client();

  void SetTimeout(duration_ms_t timeout);

  template <typename T>
  void SetSocketOption(zmqpp::socket_option option, const T &value);

  template <typename T>
  T GetSocketOption(zmqpp::socket_option option);
  template <typename T>
  void GetSocketOption(zmqpp::socket_option option, T *value);

  void Send(const std::string &identity, zmqpp::message &msg);
  bool Recv(std::string *command_out, std::string *identity_out,
            zmqpp::message &msg);

 private:
  void ConnectToBroker();

  zmqpp::context *ctx = nullptr;  //  Our context
  zmqpp::socket *sock = nullptr;  //  Socket to broker
  std::string broker;             //  Broker address
  duration_ms_t timeout;          //  Request timeout
  bool verbose;                   //  Print activity to stdout
};
}

#endif  // __MDP_CLIENT_H__
