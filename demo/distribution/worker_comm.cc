#include "worker_comm.h"

namespace bdm {

WorkerCommunicator::WorkerCommunicator(DistSharedInfo* info,
                                       const std::string& endpoint,
                                       CommunicatorId comm_id)
    : Communicator(info, endpoint, comm_id),
      logger_("WComm_[" + info_->identity_ + "]", info_->logging_level_) {
  // By convention we define that we act as client if
  // we initiate the communication with the left worker
  this->client_ = (comm_id == CommunicatorId::kLeftNeighbour ? true : false);
  this->worker_str_ =
      (client_ ? "right (client/dealer)" : "left (server/router)");
  this->coworker_str_ =
      (!client_ ? "right (client/dealer)" : "left (server/router)");
}

WorkerCommunicator::~WorkerCommunicator() {
  if (socket_) {
    info_->reactor_.remove(*socket_);
  }
}

void WorkerCommunicator::HandleOutgoingMessage(
    std::unique_ptr<zmqpp::message> msg) {
  logger_.Debug("Sending message to ", worker_str_, " worker: ", *msg);
  // TODO(kkanellis): fix this! It can be report or request
  SendToCoWorker(WorkerProtocolCmd::kReport, std::move(msg));
}

void WorkerCommunicator::HandleIncomingMessage() {
  // Expected message format
  // Frame *:     coworker_id (if client)
  // Frame 1:     "BDM/0.1W"
  // Frame 2:     WorkerCommandHeader (serialized)
  // Frame 3..n:  application frames

  auto msg_p = std::make_unique<zmqpp::message>();
  if (!socket_->receive(*msg_p)) {
    // Interrupted
    info_->zctx_interrupted_ = true;
    return;
  }
  assert(msg_p->parts() >= (2 + ((unsigned)!client_)));

  logger_.Debug("Received message from ", coworker_identity_, " worker: ",
                *msg_p);

  if (!client_) {
    // Check message origin
    std::string coworker = msg_p->get(0);
    msg_p->pop_front();
    assert(coworker == coworker_identity_   // After handshake
           || coworker_identity_.empty());  // At handshake
  }

  // Frame 1
  std::string protocol = msg_p->get(0);
  msg_p->pop_front();
  assert(protocol == MDPW_WORKER);

  // Frame 2
  std::unique_ptr<WorkerCommandHeader> header =
      WorkerCommandHeader::Deserialize(msg_p->raw_data(0), msg_p->size(0));
  msg_p->pop_front();
  assert(header->sender_ == (client_ ? CommunicatorId::kRightNeighbour
                                     : CommunicatorId::kLeftNeighbour));

  switch (header->cmd_) {
    case WorkerProtocolCmd::kReady:
      // Save coworker info
      assert(!header->client_id_.empty());
      coworker_identity_ = header->client_id_;

      if (!client_ && !is_connected_) {
        // Reply with MDPW_READY to co-worker
        logger_.Info("Connection request from ", coworker_identity_);
        SendToCoWorker(WorkerProtocolCmd::kReady);
      }
      is_connected_ = true;
      break;
    case WorkerProtocolCmd::kRequest:
    case WorkerProtocolCmd::kReport:
      // Proccess request/report from coworker
      // This should be halo-region cells request/report
      assert(coworker_identity_ == header->client_id_);

      msg_p->push_front(ToUnderlying(comm_id_));
      info_->pending_.push_back(std::move(msg_p));
      break;
    default:
      logger_.Error("Invalid input message", *msg_p);
  }
}

void WorkerCommunicator::Connect() {
  ///  W2        W3           W4         W5
  /// ... ||  L ---- R  || L ---- R  || ...
  ///  ^------^      ^-----^      ^------^
  ///  D      R      D     R      D      R

  if (client_) {
    // client initiates the communication
    socket_ = std::make_unique<zmqpp::socket>(*(info_->ctx_),
                                              zmqpp::socket_type::dealer);
    socket_->set(zmqpp::socket_option::identity, info_->identity_);
    socket_->connect(endpoint_);

    // Connect to coworker
    logger_.Info("Connecting to ", coworker_str_, " worker at ", endpoint_);
    SendToCoWorker(WorkerProtocolCmd::kReady);
  } else {
    socket_ = std::make_unique<zmqpp::socket>(*(info_->ctx_),
                                              zmqpp::socket_type::router);
    socket_->bind(endpoint_);
  }

  // Add newly created broker socket to reactor
  info_->reactor_.add(
      *socket_, std::bind(&WorkerCommunicator::HandleIncomingMessage, this));
}

void WorkerCommunicator::SendToCoWorker(
    const WorkerProtocolCmd command,
    std::unique_ptr<zmqpp::message> message /* = nullptr */) {
  // Message format sent
  // Frame 1:    BDM/0.1W
  // Frame 2:    WorkerCommandHeader class (serialized)
  // Frame 3..n: Application frames

  auto msg = message ? std::move(message) : std::make_unique<zmqpp::message>();

  auto sender = (client_ ? CommunicatorId::kLeftNeighbour
                         : CommunicatorId::kRightNeighbour);
  auto receiver = (client_ ? CommunicatorId::kRightNeighbour
                           : CommunicatorId::kLeftNeighbour);

  // Frame 2
  size_t header_sz;
  std::unique_ptr<const char[]> header =
      WorkerCommandHeader(command, sender, receiver)
          .client_id(info_->identity_)
          .worker_id(coworker_identity_)
          .Serialize(&header_sz);
  msg->push_front(header.get(), header_sz);

  // Frame 1
  msg->push_front(MDPW_WORKER);

  if (!client_) {
    // Need to add the coworker identity
    // ROUTER -> DEALER socket
    msg->push_front(coworker_identity_);
  }

  logger_.Debug("Sending ", command, " to ", coworker_identity_, " worker: ",
                *msg);
  socket_->send(*msg);
}
}  // namespace bdm
