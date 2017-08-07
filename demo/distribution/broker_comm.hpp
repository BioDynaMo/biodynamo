#ifndef __BROKER_COMM__
#define __BROKER_COMM__

#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "common.hpp"

namespace mdp {

class BrokerCommunicator {
  public:
    BrokerCommunicator (DistSharedInfo *info, const std::string& endpoint);
    ~BrokerCommunicator ();

    void RequestTimedOut();
    void RequestCompleted();

    void HandleOutgoingMessage(zmqpp::message& msg);
    void HandleIncomingMessage();

    void SetHeartbeatDelay(const duration_ms_t& hb_delay);
    void SetHeartbeatReconnect(const duration_ms_t& hb_rec_delay);

    template<typename T>
    void SetSocketOption(const zmqpp::socket_option& option, const T& value);

    template<typename T>
    T GetSocketOption(const zmqpp::socket_option& option);
    template<typename T>
    void GetSocketOption(const zmqpp::socket_option& option, T *value);


  private:
    void ConnectToBroker();
    void SendToBroker(const std::string& command, zmqpp::message *message = nullptr,
            const std::string& option = "");

    //zmqpp::socket* GetSocket (const std::uint8_t socket);

    DistSharedInfo* info_;

    zmqpp::socket *socket_ = nullptr;    //  Socket reference
    std::string endpoint_;             // 

    std::vector<zmqpp::socket*> purge_later_;

    //  Heartbeat management
    time_point_t hb_at_;             //  When to send HEARTBEAT
    duration_ms_t hb_delay_;         //  Heartbeat delay, msecs
    duration_ms_t hb_rec_delay_;     //  Reconnect delay, msecs
    size_t hb_liveness_;             //  How many attempts left

};

}

#endif //__BROKER_COMM__
