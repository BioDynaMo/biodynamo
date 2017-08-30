#ifndef DEMO_DISTRIBUTION_WORKER_ENTRY_H__
#define DEMO_DISTRIBUTION_WORKER_ENTRY_H__

#include <chrono>
#include <string>

#include "common.h"
#include "logger.h"
#include "protocol.h"

namespace bdm {

class WorkerEntry {
 public:
  explicit WorkerEntry(zmqpp::socket* socket, const std::string& identity,
                       const Logger& logger)
      : identity_(identity), socket_(socket), logger_(logger) {
    this->expiry_ = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
  }
  ~WorkerEntry() {}

  void Send(WorkerProtocolCmd command,
            std::unique_ptr<zmqpp::message> msg = nullptr,
            const std::string& client_id = "") const {
    if (msg == nullptr) {
      msg = std::make_unique<zmqpp::message>();
    }

    // Message format:
    // Frame 1:     worker_id (manually; ROUTER socket)
    // Frame 2:     "BDM/0.1W"
    // Frame 3:     WorkerMiddlewareMessageHeader class (serialized)
    // Frame 4..n:  Application frames

    // Frame 3
    auto sender =
        client_id.empty() ? CommunicatorId::kBroker : CommunicatorId::kClient;

    auto header = WorkerMiddlewareMessageHeader(command, sender,
                                                CommunicatorId::kSomeWorker)
                      .worker_id(identity_)
                      .client_id(client_id);
    MessageUtil::PushFrontHeader(msg.get(), header);

    // Frame 2
    msg->push_front(PROTOCOL_WORKER);

    // Frame 1: Deliver to correct worker
    msg->push_front(identity_);

    logger_.Debug("Sending ", command, " to worker[", identity_, "]: ", *msg);
    socket_->send(*msg);
  }

  std::string identity_;  // Worker (printable) identity
  time_point_t expiry_;   // When to send HEARTBEAT

 private:
  zmqpp::socket* socket_;
  const Logger& logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_WORKER_ENTRY_H__
