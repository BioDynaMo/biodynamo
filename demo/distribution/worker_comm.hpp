#ifndef __WORKER_COMM__
#define __WORKER_COMM__

#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "common.hpp"

namespace mdp {

class WorkerCommunicator {
  public:
    WorkerCommunicator (DistSharedInfo *info, const std::string& endpoint, bool client_);
    ~WorkerCommunicator ();

    void RequestTimedOut();
    void RequestCompleted();

    void HandleOutgoingMessage(zmqpp::message& msg);
    void HandleIncomingMessage();

    bool isConnected() {
        return is_connected_;
    }

    template<typename T>
    void SetSocketOption(const zmqpp::socket_option& option, const T& value);

    template<typename T>
    T GetSocketOption(const zmqpp::socket_option& option);
    template<typename T>
    void GetSocketOption(const zmqpp::socket_option& option, T *value);


  private:
    void ConnectToCoWorker();
    void SendToCoWorker(const std::string& command, zmqpp::message *message = nullptr,
            const std::string& option = "");

    DistSharedInfo* info_;

    zmqpp::socket *socket_ = nullptr;    // Socket reference
    std::string endpoint_;               // Where to connect?
    std::string coworker_identity_;

    bool client_;                       // Act as client? (aka initiate communication)
    std::uint8_t comm_id_;              // Type of communicator
    bool is_connected_;

    std::string coworker_str_;
    std::string worker_str_;
};

}

#endif //__WORKER_COMM__
