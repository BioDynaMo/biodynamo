#ifndef __MDP_WORKER__
#define __MDP_WORKER__

#include <string>
#include <zmqpp/zmqpp.hpp>

#include "common.hpp"

namespace mdp {

class Worker {
  public:
    Worker (zmqpp::context *ctx, const std::string& broker,
                                const std::string& identity, bool verbose);
    ~Worker();

    bool Recv (zmqpp::message& msg, std::string **reply_to);
    void Send (const zmqpp::message& msg, const std::string& reply_to);

    void SetHeartbeatDelay(std::chrono::milliseconds hb_delay);
    void SetHeartbeatReconnect(std::chrono::milliseconds hb_rec_delay);
    
    template<typename T>
    void SetSocketOption(zmqpp::socket_option option, const T& value);

    template<typename T>
    T GetSocketOption(zmqpp::socket_option option);
    template<typename T>
    void GetSocketOption(zmqpp::socket_option option, T *value);


  private:
    void ConnectToBroker();
    void SendToBroker(const std::string& command, zmqpp::message *message = nullptr,
            const std::string& option = "");


    zmqpp::context *ctx = nullptr;          //  Our context
    zmqpp::socket *sock = nullptr;          //  Socket to broker
    zmqpp::poller poller;
    std::string broker;                     //  Broker address
    std::string identity;                   //  UID of worker
    bool verbose;                           //  Print activity to stdout

    //  Heartbeat management (maybe change to ping-pong?)
    time_point_t hb_at;             //  When to send HEARTBEAT
    duration_ms_t hb_delay;         //  Heartbeat delay, msecs
    duration_ms_t hb_rec_delay;     //  Reconnect delay, msecs
    size_t hb_liveness;             //  How many attempts left

};

}

#endif //__MDP_WORKER__
