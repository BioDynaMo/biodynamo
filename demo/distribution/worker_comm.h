#ifndef DEMO_DISTRIBUTION_WORKER_COMM_H_
#define DEMO_DISTRIBUTION_WORKER_COMM_H_

#include <string>

#include "common.h"
#include "communicator.h"
#include "logger.h"
#include "protocol.h"

namespace bdm {

class WorkerCommunicator : public Communicator {
 public:
  WorkerCommunicator(DistSharedInfo* info, const std::string& endpoint,
                     CommunicatorId comm_id);
  ~WorkerCommunicator();

  void Connect() override;
  void HandleOutgoingMessage(std::unique_ptr<zmqpp::message> msg) override;
  void HandleIncomingMessage();

 private:
  void SendToCoWorker(const WorkerProtocolCmd command,
                      std::unique_ptr<zmqpp::message> message = nullptr);

  bool client_;  // Act as client? (aka initiate communication)
  std::string coworker_identity_;

  std::string coworker_str_;
  std::string worker_str_;

  Logger logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_WORKER_COMM_H_
