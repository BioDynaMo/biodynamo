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
#include <utility>
#include <vector>

#include "broker_comm.h"
#include "common.h"
#include "logger.h"
#include "worker_comm.h"

namespace bdm {

typedef std::pair<std::unique_ptr<zmqpp::message>,
                  std::unique_ptr<AppMessageHeader> >
    MessageAppHeaderPair;

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

  void SendDebugMessage(const std::string& value, CommunicatorId to);

  bool ReceiveDebugMessage(std::string* value, CommunicatorId from);

  bool ReceiveDebugMessageFromAny(std::string* value, CommunicatorId* from);

  void SetWaitingTimeout(duration_ms_t timeout) { wait_timeout_ = timeout; }

  bool IsConnected(const CommunicatorId comm) const {
    if (!IsValidCommunicator(comm)) {
      return false;
    }
    return GetValidCommunicator(comm).IsConnected();
  }

  bool Stop(bool wait = true, bool force = false);

  std::exception_ptr GetLastException() { return eptr_; }

 private:
  void SendRawMessage(std::unique_ptr<zmqpp::message> msg, CommunicatorId to);

  bool WaitForMessage(std::unique_ptr<zmqpp::message>* msg, CommunicatorId from,
                      std::unique_ptr<AppMessageHeader>* header = nullptr,
                      AppProtocolCmd cmd = AppProtocolCmd::kInvalid);

  bool WaitForMessageFromAny(
      std::unique_ptr<zmqpp::message>* msg, CommunicatorId* from,
      std::unique_ptr<AppMessageHeader>* header = nullptr,
      AppProtocolCmd cmd = AppProtocolCmd::kInvalid);

  void HandleNetwork();

  void HandleAppMessage();

  void HandleNetworkMessages();

  void Cleanup();

  inline bool IsValidCommunicator(CommunicatorId comm_id) const {
    std::uint8_t id = ToUnderlying(comm_id);
    if (id == 0 || id >= comms_.size()) {
      logger_.Error("Invalid communicator id: ", id);
      assert(false);
    }
    return (comms_[id] != nullptr);
  }

  inline Communicator& GetValidCommunicator(CommunicatorId comm_id) const {
    assert(IsValidCommunicator(comm_id));
    return *comms_[ToUnderlying(comm_id)];
  }

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
  std::array<std::deque<MessageAppHeaderPair>,
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

  duration_ms_t wait_timeout_;
  bool zctx_interrupted_;  // ZMQ interrupted by signal

  Logger logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_DIST_WORKER_API_H_
