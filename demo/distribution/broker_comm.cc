#include "broker_comm.h"

namespace bdm {

BrokerCommunicator::BrokerCommunicator(DistSharedInfo* info,
                                       const std::string& endpoint)
    : Communicator(info, endpoint, CommunicatorId::kBroker),
      hb_delay_(duration_ms_t(HEARTBEAT_INTERVAL)),
      hb_rec_delay_(duration_ms_t(HEARTBEAT_INTERVAL)) {}

BrokerCommunicator::~BrokerCommunicator() {
  SendToBroker(WorkerProtocolCmd::kDisconnect);
}

void BrokerCommunicator::ReactorTimedOut() {
  // Timeout
  if (--hb_liveness_ == 0) {
    std::cout << "W: disconnected from broker - retrying..." << std::endl;
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
    delete socket_p;
  }
  purge_later_.clear();
}

void BrokerCommunicator::HandleOutgoingMessage(
    std::unique_ptr<zmqpp::message> msg) {
  // Message Format
  // Frame 1:     client_id
  // Frame 2..n:  application frames

  if (info_->verbose_) {
    std::cout << "I: sending message to broker: " << *msg << std::endl;
  }

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

  auto msg_p = new zmqpp::message();
  if (!socket_->receive(*msg_p)) {
    // Interrupted
    info_->zctx_interrupted_ = true;
    return;
  }
  assert(msg_p->parts() >= 2);

  if (info_->verbose_) {
    std::cout << "I: received message from broker: " << *msg_p << std::endl;
  }
  hb_liveness_ = HEARTBEAT_LIVENESS;

  // Frame 1
  std::string protocol = msg_p->get(0);
  msg_p->pop_front();
  assert(protocol == MDPW_WORKER);

  // Frame 2
  std::string* header_str = new std::string(msg_p->get(0));
  msg_p->pop_front();
  std::unique_ptr<WorkerCommandHeader> header =
      ClientCommandHeader::FromString<WorkerCommandHeader>(header_str);

  switch (header->cmd_) {
    case WorkerProtocolCmd::kRequest:
      // Process message from broker
      msg_p->push_front(ToUnderlying(comm_id_));
      info_->pending_->push_back(std::unique_ptr<zmqpp::message>(msg_p));
      break;
    case WorkerProtocolCmd::kHeartbeat:
      // Do nothing
      break;
    case WorkerProtocolCmd::kDisconnect:
      // Reconnect to broker
      Connect();
      break;
    default:
      std::cout << "E: invalid input message" << *msg_p << std::endl;
  }
}

void BrokerCommunicator::Connect() {
  if (socket_) {
    // Lazy remove socket
    info_->reactor_->remove(*socket_);
    purge_later_.push_back(socket_);
  }

  // Create new socket
  socket_ = new zmqpp::socket(*(info_->ctx_), zmqpp::socket_type::dealer);
  socket_->set(zmqpp::socket_option::identity, info_->identity_);
  socket_->connect(endpoint_);

  // Add newly created broker socket to reactor
  info_->reactor_->add(
      *socket_, std::bind(&BrokerCommunicator::HandleIncomingMessage, this));

  // Register service with broker
  std::cout << "I: connecting to broker at " << endpoint_ << std::endl;
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
  std::unique_ptr<std::string> header =
      WorkerCommandHeader(command, CommunicatorId::kSomeWorker,
                          CommunicatorId::kBroker)
          .worker_id(info_->identity_)
          .client_id(client_id)
          .ToString();
  msg.push_front(*header);

  // Frame 1
  msg.push_front(MDPW_WORKER);

  if (info_->verbose_) {
    // TODO(kkanellis): change to strings
    std::cout << "I: sending " << +ToUnderlying(command)
              << " to broker: " << msg << std::endl;
  }
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
