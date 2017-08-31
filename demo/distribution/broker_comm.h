#ifndef DEMO_DISTRIBUTION_BROKER_COMM_H_
#define DEMO_DISTRIBUTION_BROKER_COMM_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include "common.h"
#include "communicator.h"
#include "logger.h"
#include "protocol.h"

namespace bdm {

/// @brief Concrete implementation of Communicator class which is responsible
///        of managing the connection to the broker.
class BrokerCommunicator : public Communicator {
 public:
  ///
  /// @brief Creates a new broker communicator
  ///
  /// @param[in]  info      pointer to shared variables needed by the API
  /// @param[in]  endpoint  broker's endpoint (i.e. "tcp://ip:port")
  ///
  BrokerCommunicator(DistSharedInfo* info, const std::string& endpoint);

  ///
  /// @brief Disconnects from the broker (if previously connected)
  ///
  /// It sends a DISCONNECT message to the broker. The broker deletes the entry
  /// for this worker. In addition, the socket is closed properly.
  ~BrokerCommunicator();

  ///
  /// @brief Callback when the reactor times out
  ///
  /// Keeps track of how many times the broker didn't respond with HEARTBEAT.
  /// If we miss HEARTBEAT exactly #HEARTBEAT_LIVENESS times, we assume that
  /// the broker is offline, and we try to reconnect.
  ///
  void ReactorTimedOut() override;

  ///
  /// @brief Callback when the reactor has just served a request (either from
  ///        the app or from the network)
  ///
  /// Checks if we (the worker) must send a HEARTBEAT to the broker. This will
  /// informs the broker that we are online and ready for requests.
  ///
  void ReactorServedRequests() override;

  ///
  /// @brief Initiates the connection procedure to the broker
  ///
  /// This procedure consists of discarding the previous connection (if any),
  /// creating a new one (over a new zmq socket) and sending kReady to broker.
  ///
  void Connect() override;

  ///
  /// @brief Sends a message through the network to the broker (or to a client)
  ///
  /// If the message is destined to a client, its id is obtained from the queue.
  ///
  /// @param[in]  msg   message to be send over the network
  ///
  void HandleOutgoingMessage(std::unique_ptr<zmqpp::message> msg) override;

  /// @brief Receive all available messages from the broker (via network)
  ///
  /// This method is called internally by the reactor when the zmq socket
  /// has available messages.
  ///
  /// For each received message, does the following:
  ///   * If the message command is #MiddlewareMessageHeader::kRequest, it is
  ///     propagated to the API (using the  DistSharedInfo::mq_app_deliver_),
  ///     in order to be deliverd to the application
  ///   * Otherwise the message is handled internally, depending on the command
  ///
  void HandleIncomingMessages() override;

  ///
  /// @brief Sets the delay between HEARTBEAT messages, sent to broker
  ///
  /// @param[in]  hb_delay  duration in milliseconds
  ///
  void SetHeartbeatDelay(const duration_ms_t& hb_delay);

  ///
  /// @brief Sets the delay before trying to reconnect to the broker, after
  ///         a disconnect has occured
  ///
  /// @param[in]  hb_rec_delay  duration in milliseconds
  ///
  void SetHeartbeatReconnect(const duration_ms_t& hb_rec_delay);

 private:
  ///
  /// @brief Constructs the middleware header and sends the message to broker
  ///
  /// If an invalid pointer to message is provided, an empty one will be created
  ///
  /// @param[in]  command     command of the message
  /// @param[in]  message     message to be sent or nullptr
  /// @param[in]  client_id   identity of client if the message is destined
  ///                         to a client; empty string otherwise
  ///
  void SendToBroker(const WorkerProtocolCmd command,
                    std::unique_ptr<zmqpp::message> message = nullptr,
                    const std::string client_id = "");

  /// Invalid zmq socket storage. Will be deleted at the first chance.
  std::vector<std::unique_ptr<zmqpp::socket> > purge_later_;

  /// Identity of clients from messages waiting to be replied.
  std::queue<std::string> clients_;

  //  Heartbeat management
  time_point_t hb_at_;          //!< When to send the next HEARTBEAT
  duration_ms_t hb_delay_;      //!< Heartbeat delay in msecs
  duration_ms_t hb_rec_delay_;  //!< Reconnect delay after disconnect in msecs
  size_t hb_liveness_;          //!< Attempts left to assume broker is offline

  Logger logger_;  //!< Logging object for this class
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BROKER_COMM_H_
