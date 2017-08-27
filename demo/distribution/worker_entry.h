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
            std::unique_ptr<zmqpp::message> message = nullptr,
            const std::string& client_id = "") const {
    auto msg = message ? message->copy() : zmqpp::message();

    // Message format:
    // Frame 1:     worker_id (manually; ROUTER socket)
    // Frame 2:     "BDM/0.1W"
    // Frame 3:     WorkerCommandHeader class (serialized)
    // Frame 4..n:  Application frames

    // Frame 3
    auto sender =
        client_id.empty() ? CommunicatorId::kBroker : CommunicatorId::kClient;

    size_t header_sz;
    std::unique_ptr<const char[]> header =
        WorkerCommandHeader(command, sender, CommunicatorId::kSomeWorker)
            .worker_id(identity_)
            .client_id(client_id)
            .Serialize(&header_sz);
    msg.push_front(header.get(), header_sz);

    // Frame 2
    msg.push_front(MDPW_WORKER);

    // Frame 1: Deliver to correct worker
    msg.push_front(identity_);

    logger_.Debug("Sending ", command, " to worker: ", msg);
    socket_->send(msg);
  }

  std::string identity_;  // Worker (printable) identity
  time_point_t expiry_;   // When to send HEARTBEAT

 private:
  zmqpp::socket* socket_;
  const Logger& logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_WORKER_ENTRY_H__
