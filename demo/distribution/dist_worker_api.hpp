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
    void StartThread(const std::string& app_addr);

    void SetBrokerEndpoint(const std::string& endpoint);
    void SetLeftNeighbourEndpoint(const std::string& endpoint);
    void SetRightNeighbourEndpoint(const std::string& endpoint);

  private:
    void HandleNetwork();
    void HandleAppMessage();
    void HandleNetworkMessages();

    DistSharedInfo *info_;

    BrokerCommunicator *broker_comm_ = nullptr;     //  Handles the connection to master
    WorkerCommunicator *lworker_comm_ = nullptr;     //  Handles the connection to left worker
    WorkerCommunicator *rworker_comm_ = nullptr;     //  Handles the connection to right worker

    std::string broker_endpoint_;
    std::string lworker_endpoint_;
    std::string rworker_endpoint_;

    std::thread *thread_ = nullptr;                 //  Background/Network thread
};

}

#endif // __DIST_WORKER_API_H
