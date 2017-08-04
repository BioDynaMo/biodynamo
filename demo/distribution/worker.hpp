#ifndef __MDP_WORKER__
#define __MDP_WORKER__

#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "common.hpp"

namespace mdp {

class Worker {
  public:
    Worker (zmqpp::context *ctx, const std::string& broker_addr,
                                const std::string& identity, bool verbose);
    ~Worker();

    void StartThread(const std::string& app_addr);

    void SetHeartbeatDelay(const duration_ms_t& hb_delay);
    void SetHeartbeatReconnect(const duration_ms_t& hb_rec_delay);

    template<typename T>
    void SetSocketOption(const zmqpp::socket_option& option, const T& value);

    template<typename T>
    T GetSocketOption(const zmqpp::socket_option& option);
    template<typename T>
    void GetSocketOption(const zmqpp::socket_option& option, T *value);


  private:
    void HandleNetwork();
    void HandleAppMessage();
    void HandleBrokerMessage();

    void ConnectToBroker();
    void SendToBroker(const std::string& command, zmqpp::message *message = nullptr,
            const std::string& option = "");

    zmqpp::reactor reactor;                 //  Polling handler
    zmqpp::context *ctx = nullptr;          //  ZMQ context
    zmqpp::socket *broker_sock = nullptr;   //  Socket to broker
    zmqpp::socket *app_sock = nullptr;      //  Socket to app

    std::vector<zmqpp::socket*> purge_later;
    std::thread *worker_thread = nullptr;   //  Background/Network thread
    std::string broker_addr;                //  Broker address
    std::string app_addr;                   //  Application address
    std::string identity;                   //  UID of worker
    bool verbose;                           //  Print activity to stdout

    //  Heartbeat management
    time_point_t hb_at;             //  When to send HEARTBEAT
    duration_ms_t hb_delay;         //  Heartbeat delay, msecs
    duration_ms_t hb_rec_delay;     //  Reconnect delay, msecs
    size_t hb_liveness;             //  How many attempts left

    bool zctx_interrupted = false;  // ZMQ interrupted by signal

};

}

#endif //__MDP_WORKER__
