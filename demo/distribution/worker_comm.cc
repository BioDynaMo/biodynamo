#include "worker_comm.h"

namespace bdm {

WorkerCommunicator::WorkerCommunicator(DistSharedInfo* info,
                                       const std::string& endpoint,
                                       CommunicatorId comm_id)
    : Communicator(info, endpoint, comm_id) {
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
    info_->reactor_->remove(*socket_);
    socket_->disconnect(endpoint_);
  }
}

void WorkerCommunicator::HandleOutgoingMessage(
    std::unique_ptr<zmqpp::message> msg) {
  if (info_->verbose_) {
    std::cout << "I: sending message to " << worker_str_ << " worker: " << *msg
              << std::endl;
  }
  SendToCoWorker(WorkerProtocolCmd::kReport, std::move(msg));
}

void WorkerCommunicator::HandleIncomingMessage() {
  // Expected message format
  // Frame *:     coworker_id (if client)
  // Frame 1:     "BDM/0.1W"
  // Frame 2:     WorkerCommandHeader (serialized)
  // Frame 3..n:  application frames

  auto msg_p = new zmqpp::message();
  if (!socket_->receive(*msg_p)) {
    // Interrupted
    info_->zctx_interrupted_ = true;
    return;
  }
  assert(msg_p->parts() >= (2 + ((unsigned)!client_)));

  if (info_->verbose_) {
    std::cout << "I: received message from " << coworker_identity_
              << " worker: " << *msg_p << std::endl;
  }

  if (!client_) {
    // Check message origin
    std::string coworker = msg_p->get(0);
    msg_p->pop_front();
    assert(coworker == coworker_identity_ || coworker_identity_.empty());
  }

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
    case WorkerProtocolCmd::kReady:
      // Save coworker info
      assert(!header->client_id_.empty());
      coworker_identity_ = header->client_id_;

      if (!client_ && !is_connected_) {
        // Reply with MDPW_READY to co-worker
        std::cout << "I: connection request from " << coworker_identity_
                  << std::endl;
        SendToCoWorker(WorkerProtocolCmd::kReady, nullptr, coworker_identity_);
      }
      is_connected_ = true;
      break;
    case WorkerProtocolCmd::kRequest:
    case WorkerProtocolCmd::kReport:
      // Proccess request/report from coworker
      // This should be halo-region cells request/report
      msg_p->push_front(ToUnderlying(comm_id_));
      info_->pending_->push_back(std::unique_ptr<zmqpp::message>(msg_p));
      break;
    default:
      std::cout << "E: invalid input message" << *msg_p << std::endl;
  }
}

void WorkerCommunicator::Connect() {
  ///  W2        W3           W4         W5
  /// ... ||  L ---- R  || L ---- R  || ...
  ///  ^------^      ^-----^      ^------^
  ///  D      R      D     R      D      R

  if (client_) {
    // client initiates the communication
    socket_ = new zmqpp::socket(*(info_->ctx_), zmqpp::socket_type::dealer);
    socket_->set(zmqpp::socket_option::identity, info_->identity_);
    socket_->connect(endpoint_);

    // Connect to coworker
    std::cout << "I: connecting to " << coworker_str_ << " worker at "
              << endpoint_ << std::endl;
    SendToCoWorker(WorkerProtocolCmd::kReady, nullptr, info_->identity_);
  } else {
    socket_ = new zmqpp::socket(*(info_->ctx_), zmqpp::socket_type::router);
    socket_->bind(endpoint_);
  }

  // Add newly created broker socket to reactor
  info_->reactor_->add(
      *socket_, std::bind(&WorkerCommunicator::HandleIncomingMessage, this));
}

void WorkerCommunicator::SendToCoWorker(
    const WorkerProtocolCmd command,
    std::unique_ptr<zmqpp::message> message /* = nullptr */,
    const std::string& client_id /* = "" */) {
  // Message format sent
  // Frame 1:    BDM/0.1W
  // Frame 2:    WorkerCommandHeader class (serialized)
  // Frame 3..n: Application frames

  auto msg = message ? message->copy() : zmqpp::message();

  auto sender = (client_ ? CommunicatorId::kLeftNeighbour
                         : CommunicatorId::kRightNeighbour);
  auto receiver = (client_ ? CommunicatorId::kRightNeighbour
                           : CommunicatorId::kLeftNeighbour);

  // Frame 2
  std::unique_ptr<std::string> header =
      WorkerCommandHeader(command, sender, receiver)
          .worker_id(info_->identity_)
          .client_id(client_id)
          .ToString();
  msg.push_front(*header);

  // Frame 1
  msg.push_front(MDPW_WORKER);

  if (!client_) {
    // Need to add the coworker identity
    // DEALER -> ROUTER socket
    msg.push_front(coworker_identity_);
  }

  if (info_->verbose_) {
    std::cout << "I: sending " << +ToUnderlying(command) << " to "
              << coworker_identity_ << " worker: " << msg << std::endl;
  }
  socket_->send(msg);
}
}  // namespace bdm
