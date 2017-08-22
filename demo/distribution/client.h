#ifndef DEMO_DISTRIBUTION_CLIENT_H_
#define DEMO_DISTRIBUTION_CLIENT_H_

#include <string>

#include "common.h"
#include "protocol.h"

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

  void Send(const std::string &identity, std::unique_ptr<zmqpp::message> msg);
  bool Recv(std::unique_ptr<zmqpp::message> *msg_out,
            ClientProtocolCmd *command_out = nullptr,
            std::string *identity_out = nullptr);

 private:
  void ConnectToBroker();

  zmqpp::context *ctx = nullptr;  //  Our context
  zmqpp::socket *sock = nullptr;  //  Socket to broker
  std::string broker;             //  Broker address
  duration_ms_t timeout;          //  Request timeout
  bool verbose;                   //  Print activity to stdout
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_CLIENT_H_
