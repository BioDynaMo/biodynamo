#include "client.h"

namespace bdm {

Client::Client(zmqpp::context* ctx, const std::string& identity,
               const std::string& broker, LoggingLevel level)
    : ctx_(ctx),
      socket_(nullptr),
      identity_(identity),
      broker_endpoint_(broker),
      timeout_(duration_ms_t(2500)),
      logger_("Client", level) {
  ConnectToBroker();
}

Client::~Client() {}

void Client::ConnectToBroker() {
  if (socket_) {
    // delete socket
    socket_->close();
    socket_.reset();
  }

  socket_ = std::make_unique<zmqpp::socket>(*ctx_, zmqpp::socket_type::dealer);
  socket_->set(zmqpp::socket_option::identity, identity_);
  socket_->connect(broker_endpoint_);

  logger_.Info("Connecting to broker at ", broker_endpoint_);
}

void Client::SetTimeout(duration_ms_t timeout) { timeout_ = timeout; }

template <typename T>
void Client::SetSocketOption(zmqpp::socket_option option, const T& value) {
  socket_->set(option, value);
}

template <typename T>
T Client::GetSocketOption(zmqpp::socket_option option) {
  T value;
  GetSocketOption(option, &value);
  return value;
}

template <typename T>
void Client::GetSocketOption(zmqpp::socket_option option, T* value) {
  socket_->get(option, *value);
}

void Client::Send(ClientProtocolCmd cmd, CommunicatorId receiver,
                  std::unique_ptr<zmqpp::message> msg,
                  const std::string& worker_id) {
  //  Message format:
  //  Frame 1:    "BDM/0.1C"
  //  Frame 2:    ClientCommandHeader class (serialized)
  //  Frame 3..n: Application frames

  if (!msg) {
    msg = std::make_unique<zmqpp::message>();
  }

  assert(receiver == CommunicatorId::kBroker ||
         receiver == CommunicatorId::kSomeWorker);

  // Frame 2
  size_t header_sz;
  std::unique_ptr<const char[]> header =
      ClientCommandHeader(cmd, CommunicatorId::kClient, receiver)
          .client_id(identity_)
          .worker_id(worker_id)
          .Serialize(&header_sz);
  msg->push_front(header.get(), header_sz);

  // Frame 1
  msg->push_front(MDPC_CLIENT);

  if (receiver == CommunicatorId::kBroker) {
    logger_.Debug("Send ", cmd, " request to broker: ", *msg);
  } else {
    logger_.Debug("Send request to worker [", worker_id, "]: ", *msg);
  }
  socket_->send(*msg);
}

//  Here is the send method. It sends a request to the broker.
//  It takes ownership of the request message, and destroys it when sent.
void Client::SendToWorker(std::unique_ptr<zmqpp::message> msg,
                          const std::string& worker_id) {
  // Worker identity must not be empty
  assert(!worker_id.empty());

  Send(ClientProtocolCmd::kRequest, CommunicatorId::kSomeWorker, std::move(msg),
       worker_id);
}

bool Client::Recv(std::unique_ptr<zmqpp::message>* msg_out,
                  ClientProtocolCmd* command /* = nullptr */,
                  std::string* recv_from /* = nullptr */) {
  auto msg = std::make_unique<zmqpp::message>();
  if (!socket_->receive(*msg)) {
    return false;
  }

  logger_.Debug("Received reply: ", *msg);

  //  Message format:
  //  Frame 1:    "BDM/0.1C"
  //  Frame 2:    ClientCommandHeader class (serialized)
  //  Frame 3..n: Application frames
  assert(msg->parts() >= 2);

  // Read protocol version
  std::string protocol = msg->get(0);
  msg->pop_front();
  assert(protocol == MDPC_CLIENT);

  std::unique_ptr<ClientCommandHeader> header =
      ClientCommandHeader::Deserialize(msg->raw_data(0), msg->size(0));
  msg->pop_front();
  assert(header->cmd_ == ClientProtocolCmd::kReport ||
         header->cmd_ == ClientProtocolCmd::kAck ||
         header->cmd_ == ClientProtocolCmd::kNak);

  if (command != nullptr) {
    *command = header->cmd_;
  }

  if (recv_from != nullptr) {
    *recv_from = header->worker_id_;
  }

  *msg_out = std::move(msg);
  return true;
}

bool Client::CheckWorker(const std::string& worker_id) {
  logger_.Debug("Checking worker [", worker_id, "]...");
  Send(ClientProtocolCmd::kCheckWorker, CommunicatorId::kBroker, nullptr,
       worker_id);

  auto msg = std::make_unique<zmqpp::message>();
  ClientProtocolCmd command;
  if (!Recv(&msg, &command)) {
    logger_.Error("Interrupted...");
    return false;
  }

  logger_.Debug("Received ", command, " from broker");
  return (command == ClientProtocolCmd::kAck);
}

bool Client::RequestBrokerTermination() {
  logger_.Debug("Send termination request to broker...");
  Send(ClientProtocolCmd::kBrokerTerminate, CommunicatorId::kBroker);

  auto msg = std::make_unique<zmqpp::message>();
  ClientProtocolCmd command;
  if (!Recv(&msg, &command)) {
    logger_.Error("Interrupted...");
    return false;
  }
  logger_.Debug("Received ", command, " from broker");
  return (command == ClientProtocolCmd::kAck);
}

}  // namespace bdm
