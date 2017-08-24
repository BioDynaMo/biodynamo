#ifndef DEMO_DISTRIBUTION_BROKER_COMM_H_
#define DEMO_DISTRIBUTION_BROKER_COMM_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "common.h"
#include "communicator.h"
#include "logger.h"
#include "protocol.h"

namespace bdm {

class BrokerCommunicator : public Communicator {
 public:
  BrokerCommunicator(DistSharedInfo* info, const std::string& endpoint);
  ~BrokerCommunicator();

  void ReactorTimedOut() override;
  void ReactorServedRequests() override;

  void Connect() override;
  void HandleOutgoingMessage(std::unique_ptr<zmqpp::message> msg) override;
  void HandleIncomingMessage() override;

  void SetHeartbeatDelay(const duration_ms_t& hb_delay);
  void SetHeartbeatReconnect(const duration_ms_t& hb_rec_delay);

 private:
  void SendToBroker(const WorkerProtocolCmd command,
                    std::unique_ptr<zmqpp::message> message = nullptr,
                    const std::string client_id = "");

  std::vector<zmqpp::socket*> purge_later_;

  //  Heartbeat management
  time_point_t hb_at_;          //  When to send HEARTBEAT
  duration_ms_t hb_delay_;      //  Heartbeat delay, msecs
  duration_ms_t hb_rec_delay_;  //  Reconnect delay, msecs
  size_t hb_liveness_;          //  How many attempts left

  Logger logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BROKER_COMM_H_
