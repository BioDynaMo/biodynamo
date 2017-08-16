#ifndef __BROKER_COMM__
#define __BROKER_COMM__

#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "common.h"
#include "communicator.h"

namespace mdp {

class BrokerCommunicator : public Communicator {
  public:
    BrokerCommunicator (DistSharedInfo *info, const std::string& endpoint);
    ~BrokerCommunicator ();

    void ReactorTimedOut() override;
    void ReactorServedRequests() override;

    void Connect();
    void HandleOutgoingMessage(zmqpp::message& msg);
    void HandleIncomingMessage();

    void SetHeartbeatDelay(const duration_ms_t& hb_delay);
    void SetHeartbeatReconnect(const duration_ms_t& hb_rec_delay);

  private:
    void SendToBroker(const std::string& command, zmqpp::message *message = nullptr,
            const std::string& option = "");

    std::vector<zmqpp::socket*> purge_later_;

    //  Heartbeat management
    time_point_t hb_at_;             //  When to send HEARTBEAT
    duration_ms_t hb_delay_;         //  Heartbeat delay, msecs
    duration_ms_t hb_rec_delay_;     //  Reconnect delay, msecs
    size_t hb_liveness_;             //  How many attempts left

};

}

#endif //__BROKER_COMM__
