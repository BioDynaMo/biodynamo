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

void WorkerCommunicator::HandleOutgoingMessage(zmqpp::message& msg) {
  if (info_->verbose_) {
    std::cout << "I: sending message to " << worker_str_ << " worker: " << msg
              << std::endl;
  }
  SendToCoWorker(MDPW_REPORT, &msg);
}

void WorkerCommunicator::HandleIncomingMessage() {
  auto msg_p = new zmqpp::message();
  if (!socket_->receive(*msg_p)) {
    // Interrupted
    info_->zctx_interrupted_ = true;
    return;
  }

  if (info_->verbose_) {
    std::cout << "I: received message from " << coworker_identity_
              << " worker: " << *msg_p << std::endl;
  }

  if (!client_) {
    // Check message origin
    std::string coworker;
    msg_p->get(coworker, 0);
    msg_p->pop_front();
    assert(coworker == coworker_identity_ || coworker_identity_.empty());
  }

  std::string empty, header, command;
  assert(msg_p->parts() >= 3);

  msg_p->get(empty, 0);
  assert(empty == "");
  msg_p->pop_front();

  msg_p->get(header, 0);
  assert(header == MDPW_WORKER);
  msg_p->pop_front();

  msg_p->get(command, 0);
  msg_p->pop_front();

  if (command == MDPW_READY) {
    // Save coworker info
    msg_p->get(coworker_identity_, 0);
    assert(!coworker_identity_.empty());
    msg_p->pop_front();

    if (!client_ && !is_connected_) {
      std::cout << "I: connection request from " << coworker_identity_
                << std::endl;

      // Reply with MDPW_READY to co-worker
      SendToCoWorker(MDPW_READY, nullptr, info_->identity_);
    }
    is_connected_ = true;
  } else if (command == MDPW_REQUEST || command == MDPW_REPORT) {
    // Proccess request/report from coworker
    // This should be halo-region cells request/report
    msg_p->push_front(ToUnderlying(comm_id_));
    info_->pending_->push_back(std::unique_ptr<zmqpp::message>(msg_p));
  } else {
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
    SendToCoWorker(MDPW_READY, nullptr, info_->identity_);
  } else {
    socket_ = new zmqpp::socket(*(info_->ctx_), zmqpp::socket_type::router);
    socket_->bind(endpoint_);
  }

  // Add newly created broker socket to reactor
  info_->reactor_->add(
      *socket_, std::bind(&WorkerCommunicator::HandleIncomingMessage, this));
}

void WorkerCommunicator::SendToCoWorker(const std::string& command,
                                        zmqpp::message* message /* = nullptr */,
                                        const std::string& option /* = "" */) {
  auto msg = message ? message->copy() : zmqpp::message();

  //  Stack protocol envelope to start of message
  if (!option.empty()) {
    msg.push_front(option);
  }
  msg.push_front(command);
  msg.push_front(MDPW_WORKER);
  msg.push_front("");

  if (!client_) {
    // Need to add the coworker identity
    // DEALER -> ROUTER socket
    msg.push_front(coworker_identity_);
  }

  if (info_->verbose_) {
    std::cout << "I: sending " << command << " to " << coworker_identity_
              << " worker: " << msg << std::endl;
  }
  socket_->send(msg);
}
}  // namespace bdm
