#ifndef __DIST_WORKER_API_H
#define __DIST_WORKER_API_H

#include <string>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "common.hpp"
#include "broker_comm.hpp"
#include "worker_comm.hpp"

namespace mdp {

class DistWorkerAPI {
  public:
    DistWorkerAPI(zmqpp::context *ctx, const std::string identity, bool verbose);
    ~DistWorkerAPI();
    bool Start();

    void AddBrokerCommunicator(const std::string& endpoint);
    void AddLeftNeighbourCommunicator(const std::string& endpoint);
    void AddRightNeighbourCommunicator(const std::string& endpoint);

    void SendMessage(zmqpp::message& msg);
    void ReceiveMessage(zmqpp::message *msg);

    bool Stop(bool wait = true, bool force = false);

    std::exception_ptr GetLastException() {
        return eptr_;
    }

  private:
    void HandleNetwork();
    void HandleAppMessage();
    void HandleNetworkMessages();
    void Cleanup();

    Communicator& GetValidCommunicator(std::uint8_t comm__id);
    void ForEachValidCommunicator(std::function<void(std::unique_ptr<Communicator>&)> f);

    DistSharedInfo *info_;

    std::vector< std::unique_ptr<Communicator> > comms_;

    zmqpp::socket *parent_pipe_;            // Used by computation thread
    zmqpp::socket *child_pipe_;             // Used by background thread
    std::string endpoint_;                  // Application endpoint

    std::thread *thread_ = nullptr;         //  Background/Network thread
    std::exception_ptr eptr_ = nullptr;     //  Holds the last exception
};

}

#endif // __DIST_WORKER_API_H
