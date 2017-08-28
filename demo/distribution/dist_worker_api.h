#ifndef DEMO_DISTRIBUTION_DIST_WORKER_API_H_
#define DEMO_DISTRIBUTION_DIST_WORKER_API_H_

#include <algorithm>
#include <array>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "broker_comm.h"
#include "common.h"
#include "logger.h"
#include "worker_comm.h"

namespace bdm {

class DistWorkerAPI {
 public:
  DistWorkerAPI(zmqpp::context* ctx, const std::string identity,
                LoggingLevel level);
  ~DistWorkerAPI();
  bool Start();

  void AddBrokerCommunicator(const std::string& endpoint);

  /// \brief Add a connection to a worker positioned left (to this worker)
  //
  //  This means that this worker is considered as RIGHT worker (by the other
  //  worker)
  void AddLeftNeighbourCommunicator(const std::string& endpoint);

  /// \brief Add a connection to a worker positioned right (to this worker)
  //
  //  This means that this worker is considered as LEFT worker (by the other
  //  worker)
  void AddRightNeighbourCommunicator(const std::string& endpoint);

  void SendMessage(std::unique_ptr<zmqpp::message> msg, CommunicatorId to);
  bool ReceiveMessage(std::unique_ptr<zmqpp::message>* msg,
                      CommunicatorId* from = nullptr,
                      duration_ms_t timeout = std::chrono::milliseconds(1000));
  bool ReceiveMessage(std::unique_ptr<zmqpp::message>* msg, CommunicatorId from,
                      duration_ms_t timeout = std::chrono::milliseconds(1000));

  bool IsConnected(const CommunicatorId comm) const;

  bool Stop(bool wait = true, bool force = false);

  std::exception_ptr GetLastException() { return eptr_; }

 private:
  void HandleNetwork();
  void HandleAppMessage();

  void HandleNetworkMessages();

  void Cleanup();

  inline bool IsValidCommunicator(CommunicatorId comm_id) const;
  inline Communicator& GetValidCommunicator(CommunicatorId comm__id) const;
  void ForEachValidCommunicator(
      std::function<void(std::unique_ptr<Communicator>&)> f);

  DistSharedInfo info_;

  // Keep all (active) communicator
  std::array<std::unique_ptr<Communicator>,
             ToUnderlying(CommunicatorId::kCount)>
      comms_;

  // Messages waiting to be handled by the communicators
  // (and then sent over the network)
  std::queue<MessageMiddlewareHeaderPair> mq_net_deliver_;
  std::mutex mq_net_deliver_mtx_;

  // Messages ready to be delivered to the application
  std::array<std::deque<std::unique_ptr<zmqpp::message> >,
             ToUnderlying(CommunicatorId::kCount)>
      app_messages_;
  // Serialize access to app_messages_
  std::condition_variable app_messages_cv_;
  std::mutex app_messages_mtx_;

  std::unique_ptr<zmqpp::socket> parent_pipe_;  // Used by computation thread
  std::unique_ptr<zmqpp::socket> child_pipe_;   // Used by background thread
  std::string endpoint_;                        // Application endpoint

  std::unique_ptr<std::thread> thread_;  //  Background/Network thread
  std::exception_ptr eptr_;              //  Holds the last exception

  bool zctx_interrupted_;  // ZMQ interrupted by signal

  Logger logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_DIST_WORKER_API_H_
