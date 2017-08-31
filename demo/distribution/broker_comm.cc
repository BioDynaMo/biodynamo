#include "broker_comm.h"

namespace bdm {

BrokerCommunicator::BrokerCommunicator(DistSharedInfo* info,
                                       const std::string& endpoint)
    : Communicator(info, endpoint, CommunicatorId::kBroker),
      hb_delay_(duration_ms_t(HEARTBEAT_INTERVAL)),
      hb_rec_delay_(duration_ms_t(HEARTBEAT_INTERVAL)),
      logger_("BComm_[" + info_->identity_ + "]", info_->logging_level_) {}

BrokerCommunicator::~BrokerCommunicator() {
  SendToBroker(WorkerProtocolCmd::kDisconnect);
}

void BrokerCommunicator::ReactorTimedOut() {
  if (--hb_liveness_ == 0) {
    // Haven't heard from the broker for a while
    is_connected_ = false;

    logger_.Warning("Disconnected from broker - retrying...");
    std::this_thread::sleep_for(hb_rec_delay_);

    // Trying to reconnect
    Connect();
  }
}

void BrokerCommunicator::ReactorServedRequests() {
  if (std::chrono::system_clock::now() > hb_at_) {
    // Send HEARTBEAT to the broker
    SendToBroker(WorkerProtocolCmd::kHeartbeat);
    hb_at_ = std::chrono::system_clock::now() + hb_delay_;
  }

  // Purge old & invalid sockets
  for (auto& socket_p : purge_later_) {
    socket_p->close();
  }
  purge_later_.clear();
}

void BrokerCommunicator::HandleOutgoingMessage(
    std::unique_ptr<zmqpp::message> msg) {
  // Message Format
  // Frame 1:     AppMessageHeader
  // Frame 2..n:  application frames

  // Send to correct client
  // NOTE: FIFO order of replying to clients
  assert(!clients_.empty());
  std::string client_id = clients_.front();
  clients_.pop();

  logger_.Debug("Sending message to client [", client_id, "] via broker: ",
                *msg);
  SendToBroker(WorkerProtocolCmd::kReport, std::move(msg), client_id);
}

void BrokerCommunicator::HandleIncomingMessages() {
  // Expected message format
  // Frame 1:     "BDM/0.1W"
  // Frame 2:     WorkerMiddlewareMessageHeader
  // Frame 3..n:  application frames

  // Receive all messages from the socket
  auto msg_p = std::make_unique<zmqpp::message>();
  while (socket_->receive(*msg_p, true)) {
    logger_.Debug("Received message from broker: ", *msg_p);

    assert(msg_p->parts() >= 2);
    hb_liveness_ = HEARTBEAT_LIVENESS;  // Broker is alive

    // Frame 1
    std::string protocol = msg_p->get(0);
    msg_p->pop_front();
    assert(protocol == PROTOCOL_WORKER);

    // Frame 2
    auto header =
        MessageUtil::PopFrontObject<WorkerMiddlewareMessageHeader>(msg_p.get());

    //
    switch (header->cmd_) {
      case WorkerProtocolCmd::kRequest:
        // Keep which client made the request
        // Since we use synchronous request, the application will reply
        // FIRST to this request (FIFO order)
        clients_.push(header->client_id_);

        // Propagate message to the API
        info_->mq_app_deliver_.push(
            std::make_pair(std::move(msg_p), CommunicatorId::kBroker));
        break;
      case WorkerProtocolCmd::kHeartbeat:
        // Update connection status
        is_connected_ = true;
        break;
      case WorkerProtocolCmd::kDisconnect:
        // Reconnect to broker
        Connect();
        break;
      default:
        logger_.Error("Invalid input message", *msg_p);
    }

    // Prepare for next message
    msg_p = std::make_unique<zmqpp::message>();
  }
}

void BrokerCommunicator::Connect() {
  if (socket_) {
    // Lazy remove socket
    info_->reactor_.remove(*socket_);
    purge_later_.push_back(std::move(socket_));
  }

  // Create new socket
  socket_ = std::make_unique<zmqpp::socket>(*(info_->ctx_),
                                            zmqpp::socket_type::dealer);
  socket_->set(zmqpp::socket_option::identity, info_->identity_);
  socket_->connect(endpoint_);

  // Add newly created broker socket to reactor
  info_->reactor_.add(
      *socket_, std::bind(&BrokerCommunicator::HandleIncomingMessages, this));

  // Register service with broker
  logger_.Info("Connecting to broker at ", endpoint_);
  SendToBroker(WorkerProtocolCmd::kReady);

  // Heartbeat
  hb_liveness_ = HEARTBEAT_LIVENESS;
  hb_at_ = std::chrono::system_clock::now() + hb_delay_;
}

void BrokerCommunicator::SendToBroker(
    const WorkerProtocolCmd command,
    std::unique_ptr<zmqpp::message> msg /* = nullptr */,
    const std::string client_id /* = "" */) {
  // Sending message structure
  // Frame 1:    BDM/0.1W
  // Frame 2:    WorkerMiddlewareMessageHeader
  // Frame 3..n: Application frames

  if (!msg) {
    msg = std::make_unique<zmqpp::message>();
  }

  auto receiver =
      client_id.empty() ? CommunicatorId::kBroker : CommunicatorId::kClient;

  // Frame 2
  auto header = WorkerMiddlewareMessageHeader(
                    command, CommunicatorId::kSomeWorker, receiver)
                    .worker_id(info_->identity_)
                    .client_id(client_id);
  MessageUtil::PushFrontObject(msg.get(), header);

  // Frame 1
  msg->push_front(PROTOCOL_WORKER);

  logger_.Debug("Sending ", command, " to broker: ", *msg);
  socket_->send(*msg);
}

void BrokerCommunicator::SetHeartbeatDelay(const duration_ms_t& hb_delay) {
  this->hb_delay_ = hb_delay_;
}

void BrokerCommunicator::SetHeartbeatReconnect(
    const duration_ms_t& hb_rec_delay) {
  this->hb_rec_delay_ = hb_rec_delay_;
}
}  // namespace bdm
