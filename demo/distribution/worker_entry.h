#ifndef DEMO_DISTRIBUTION_WORKER_ENTRY_H__
#define DEMO_DISTRIBUTION_WORKER_ENTRY_H__

#include <chrono>
#include <list>
#include <string>

#include "common.h"
#include "logger.h"
#include "protocol.h"

namespace bdm {

class WorkerEntry {
 public:
  explicit WorkerEntry(const std::string& identity, const Logger& logger)
      : logger_(logger) {
    this->identity = identity;
    this->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
  }
  ~WorkerEntry() {}

  void Send(zmqpp::socket* sock, WorkerProtocolCmd command,
            zmqpp::message* message = nullptr,
            std::string client_id = "") const {
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
            .worker_id(identity)
            .client_id(client_id)
            .Serialize(&header_sz);
    msg.push_front(header.get(), header_sz);

    // Frame 2
    msg.push_front(MDPW_WORKER);

    // Frame 1: Deliver to correct worker
    msg.push_front(identity);

    logger_.Debug("Sending ", command, " to worker: ", msg);
    sock->send(msg);
  }

  std::list<zmqpp::message> requests;  // Pending requests
  std::string identity;                // Worker (printable) identity
  time_point_t expiry;                 // When to send HEARTBEAT

 private:
  bool verbose_;
  const Logger& logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_WORKER_ENTRY_H__
