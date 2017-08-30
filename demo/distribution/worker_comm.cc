#include "worker_comm.h"

namespace bdm {

WorkerCommunicator::WorkerCommunicator(DistSharedInfo* info,
                                       const std::string& endpoint,
                                       CommunicatorId comm_id)
    : Communicator(info, endpoint, comm_id),
      logger_("WComm_[" + info_->identity_ + "]", info_->logging_level_) {
  // By convention we define that we act as client if
  // we initiate the communication with the left worker
  this->client_ = (comm_id == CommunicatorId::kRightNeighbour ? true : false);
  this->coworker_str_ =
      (client_ ? "right (server/router)" : "left (client/dealer)");
}

WorkerCommunicator::~WorkerCommunicator() {
  if (socket_) {
    info_->reactor_.remove(*socket_);
  }
}

void WorkerCommunicator::HandleOutgoingMessage(
    std::unique_ptr<zmqpp::message> msg) {
  logger_.Debug("Sending message to ", coworker_str_, " co-worker: ", *msg);

  // TODO(kkanellis): fix this! It can be report or request
  // Actually doesn't matter -- can be handled by app level protocol
  SendToCoWorker(WorkerProtocolCmd::kReport, std::move(msg));
}

void WorkerCommunicator::HandleIncomingMessage() {
  // Expected message format
  // Frame *:     coworker_id (if client)
  // Frame 1:     "BDM/0.1W"
  // Frame 2:     WorkerMiddlewareMessageHeader (serialized)
  // Frame 3..n:  application frames

  // Receive all messages from the socket
  auto msg_p = std::make_unique<zmqpp::message>();
  while (socket_->receive(*msg_p, true)) {
    assert(msg_p->parts() >= (2 + ((unsigned)!client_)));
    logger_.Debug("Received message from ", coworker_str_, " co-worker [",
                  coworker_identity_, "]: ", *msg_p);

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
    assert(protocol == PROTOCOL_WORKER);

    // Frame 2
    std::unique_ptr<WorkerMiddlewareMessageHeader> header =
        WorkerMiddlewareMessageHeader::Deserialize(msg_p->raw_data(0),
                                                   msg_p->size(0));
    msg_p->pop_front();
    assert(header->sender_ == (client_ ? CommunicatorId::kRightNeighbour
                                       : CommunicatorId::kLeftNeighbour));

    switch (header->cmd_) {
      case WorkerProtocolCmd::kReady:
        // Save coworker info
        assert(!header->client_id_.empty());
        coworker_identity_ = header->client_id_;

        if (!client_ && !is_connected_) {
          // Reply with [Ready] to co-worker
          logger_.Info("Connection request from ", coworker_str_,
                       " co-worker [", coworker_identity_, "]");
          SendToCoWorker(WorkerProtocolCmd::kReady);
        }
        is_connected_ = true;
        break;
      case WorkerProtocolCmd::kRequest:
      case WorkerProtocolCmd::kReport:
        // Proccess request/report from coworker
        // This should be halo-region cells request/report
        assert(is_connected_);
        assert(coworker_identity_ == header->client_id_);

        info_->mq_app_deliver_.push(std::make_pair(std::move(msg_p), comm_id_));
        break;
      default:
        logger_.Error("Invalid input message", *msg_p);
    }

    // Prepare for next message
    msg_p = std::make_unique<zmqpp::message>();
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
    logger_.Info("Connecting to ", coworker_str_, " co-worker [",
                 coworker_identity_, "] at ", endpoint_);
    SendToCoWorker(WorkerProtocolCmd::kReady);
  } else {
    socket_ = std::make_unique<zmqpp::socket>(*(info_->ctx_),
                                              zmqpp::socket_type::router);
    socket_->bind(endpoint_);
    logger_.Info("Waiting for connection from ", coworker_str_,
                 " co-worker at ", endpoint_);
  }
  // Add newly created broker socket to reactor
  info_->reactor_.add(
      *socket_, std::bind(&WorkerCommunicator::HandleIncomingMessage, this));

  // Maybe client already sent a messages
  // Note: it is automatically buffered by 0ZQ
  HandleIncomingMessage();
}

void WorkerCommunicator::SendToCoWorker(
    const WorkerProtocolCmd command,
    std::unique_ptr<zmqpp::message> message /* = nullptr */) {
  // Message format sent
  // Frame 1:    BDM/0.1W
  // Frame 2:    WorkerMiddlewareMessageHeader class (serialized)
  // Frame 3..n: Application frames

  auto msg = message ? std::move(message) : std::make_unique<zmqpp::message>();

  auto receiver = comm_id_;
  auto sender = (comm_id_ == CommunicatorId::kLeftNeighbour
                     ? CommunicatorId::kRightNeighbour
                     : CommunicatorId::kLeftNeighbour);

  // Frame 2
  size_t header_sz;
  std::unique_ptr<const char[]> header =
      WorkerMiddlewareMessageHeader(command, sender, receiver)
          .client_id(info_->identity_)
          .worker_id(coworker_identity_)
          .Serialize(&header_sz);
  msg->push_front(header.get(), header_sz);

  // Frame 1
  msg->push_front(PROTOCOL_WORKER);

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
