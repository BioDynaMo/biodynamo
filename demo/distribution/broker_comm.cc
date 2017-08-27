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
  // Timeout
  // TODO(kkanellis): decrease liveness IF it's time
  if (--hb_liveness_ == 0) {
    logger_.Warning("Disconnected from broker - retrying...");
    std::this_thread::sleep_for(hb_rec_delay_);
    Connect();
  }
}

void BrokerCommunicator::ReactorServedRequests() {
  //  Send HEARTBEAT if it's time
  if (std::chrono::system_clock::now() > hb_at_) {
    SendToBroker(WorkerProtocolCmd::kHeartbeat);
    hb_at_ = std::chrono::system_clock::now() + hb_delay_;
  }

  // Purge old sockets
  for (auto& socket_p : purge_later_) {
    socket_p->close();
  }
  purge_later_.clear();
}

void BrokerCommunicator::HandleOutgoingMessage(
    std::unique_ptr<zmqpp::message> msg) {
  // Message Format
  // Frame 1:     client_id
  // Frame 2..n:  application frames

  logger_.Debug("Sending message to broker: ", *msg);

  assert(msg->parts() >= 1);

  // Verify client id exists
  std::string client_id = msg->get(0);
  msg->pop_front();
  assert(!client_id.empty());

  SendToBroker(WorkerProtocolCmd::kReport, std::move(msg), client_id);
}

void BrokerCommunicator::HandleIncomingMessage() {
  // Expected message format
  // Frame 1:     "BDM/0.1W"
  // Frame 2:     WorkerCommandHeader (serialized)
  // Frame 3..n:  application frames

  // TODO(kkanellis): replace with smart pointer
  auto msg_p = std::make_unique<zmqpp::message>();
  if (!socket_->receive(*msg_p)) {
    // Interrupted
    info_->zctx_interrupted_ = true;
    return;
  }
  assert(msg_p->parts() >= 2);
  logger_.Debug("Received message from broker: ", *msg_p);

  hb_liveness_ = HEARTBEAT_LIVENESS;

  // Frame 1
  std::string protocol = msg_p->get(0);
  msg_p->pop_front();
  assert(protocol == MDPW_WORKER);

  // Frame 2
  std::unique_ptr<WorkerCommandHeader> header =
      WorkerCommandHeader::Deserialize(msg_p->raw_data(0), msg_p->size(0));
  msg_p->pop_front();

  switch (header->cmd_) {
    case WorkerProtocolCmd::kRequest:
      // Process message from broker
      msg_p->push_front(ToUnderlying(comm_id_));
      info_->mq_app_deliver_.push(
          std::make_pair(std::move(msg_p), CommunicatorId::kBroker));
      break;
    case WorkerProtocolCmd::kHeartbeat:
      // Do nothing
      break;
    case WorkerProtocolCmd::kDisconnect:
      // Reconnect to broker
      Connect();
      break;
    default:
      logger_.Error("Invalid input message", *msg_p);
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
      *socket_, std::bind(&BrokerCommunicator::HandleIncomingMessage, this));

  // Register service with broker
  logger_.Info("Connecting to broker at ", endpoint_);
  SendToBroker(WorkerProtocolCmd::kReady);

  // Heartbeat
  hb_liveness_ = HEARTBEAT_LIVENESS;
  hb_at_ = std::chrono::system_clock::now() + hb_delay_;
}

void BrokerCommunicator::SendToBroker(
    const WorkerProtocolCmd command,
    std::unique_ptr<zmqpp::message> message /* = nullptr */,
    const std::string client_id /* = "" */) {
  // Message format sent
  // Frame 1:    BDM/0.1W
  // Frame 2:    WorkerCommandHeader class (serialized)
  // Frame 3..n: Application frames

  auto msg = message ? message->copy() : zmqpp::message();

  // Frame 2
  size_t header_sz;
  std::unique_ptr<const char[]> header =
      WorkerCommandHeader(command, CommunicatorId::kSomeWorker,
                          CommunicatorId::kBroker)
          .worker_id(info_->identity_)
          .client_id(client_id)
          .Serialize(&header_sz);
  msg.push_front(header.get(), header_sz);

  // Frame 1
  msg.push_front(MDPW_WORKER);

  logger_.Debug("Sending ", command, " to broker: ", msg);
  socket_->send(msg);
}

void BrokerCommunicator::SetHeartbeatDelay(const duration_ms_t& hb_delay) {
  this->hb_delay_ = hb_delay_;
}

void BrokerCommunicator::SetHeartbeatReconnect(
    const duration_ms_t& hb_rec_delay) {
  this->hb_rec_delay_ = hb_rec_delay_;
}
}  // namespace bdm
