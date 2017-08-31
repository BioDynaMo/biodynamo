#ifndef DEMO_DISTRIBUTION_WORKER_COMM_H_
#define DEMO_DISTRIBUTION_WORKER_COMM_H_

#include <string>

#include "common.h"
#include "communicator.h"
#include "logger.h"
#include "protocol.h"

namespace bdm {

/// @brief Concrete implementation of Communicator class which is responsible
///        of managing the connection to another worker/neighbour
class WorkerCommunicator : public Communicator {
 public:
  ///
  /// @brief Creates a new worker communicator
  ///
  /// @param[in]  info      pointer to shared variables needed by the API
  /// @param[in]  endpoint  remote node endpoint if \p comm_id is
  ///                       #CommunicatorId::kRightNeighbour; this node endpoint
  ///                       otherwise.
  /// @param[in]  comm_id   #CommunicatorId::kLeftNeighbour or
  ///                       #CommunicatorId::kRightNeighbour depending on this
  ///                       node's view.
  ///
  WorkerCommunicator(DistSharedInfo* info, const std::string& endpoint,
                     CommunicatorId comm_id);

  ///
  /// @brief Disconnects from the co-worker
  ///
  /// It closes the socket and marks it as invalid
  ///
  ~WorkerCommunicator();

  ///
  /// @brief Initiates the connection procedure to the co-worker/neighbor
  ///
  /// This procedure starts with a handshaking phase. No messages can be sent
  /// over the network, until this phase is over.
  ///
  void Connect() override;

  ///
  /// @brief Sends a message through the network to the co-worker/neighbour
  ///
  /// @param[in]  msg   message to be send over the network
  ///
  void HandleOutgoingMessage(std::unique_ptr<zmqpp::message> msg) override;

  /// @brief Receive all available messages from the co-worker
  ///
  /// This method is called internally by the reactor when the zmq socket
  /// has available messages.
  ///
  /// For each received message, does the following:
  ///   * If the message command is #MiddlewareMessageHeader::kRequest or
  ///     #MiddlewareMessageHeader::kReport, it is propagated to the API
  ///     (using the  DistSharedInfo::mq_app_deliver_), in order to be deliverd
  ///     to the application
  ///   * Otherwise the message is handled internally, depending on the command
  ///
  void HandleIncomingMessages() override;

 private:
  ///
  /// @brief Constructs the middleware header and sends the message to co-worker
  ///
  /// If an invalid pointer to message is provided, an empty one will be created
  ///
  /// @param[in]  command     command of the message
  /// @param[in]  message     message to be sent or nullptr
  ///
  void SendToCoWorker(const WorkerProtocolCmd command,
                      std::unique_ptr<zmqpp::message> message = nullptr);

  bool client_;                    //!< \p true if we initiate the handshake
  std::string coworker_identity_;  //!< Co-worker's identity

  std::string coworker_str_;  //!< Co-worker's description (used for dbg)

  Logger logger_;  //!< Logging object for this class
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_WORKER_COMM_H_
