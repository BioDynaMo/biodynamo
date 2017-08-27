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

//  Here is the send method. It sends a request to the broker.
//  It takes ownership of the request message, and destroys it when sent.

void Client::Send(const std::string& worker_identity,
                  std::unique_ptr<zmqpp::message> msg) {
  //  Message format:
  //  Frame 1:    "BDM/0.1C"
  //  Frame 2:    ClientCommandHeader class (serialized)
  //  Frame 3..n: Application frames

  // Frame 2
  size_t header_sz;
  std::unique_ptr<const char[]> header =
      ClientCommandHeader(ClientProtocolCmd::kRequest, CommunicatorId::kClient,
                          CommunicatorId::kSomeWorker)
          .client_id(identity_)
          .worker_id(worker_identity)
          .Serialize(&header_sz);
  msg->push_front(header.get(), header_sz);

  // Frame 1
  msg->push_front(MDPC_CLIENT);

  logger_.Debug("Send request to '", worker_identity, "' : ", *msg);
  socket_->send(*msg);
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

void Client::RequestBrokerTermination() {
  zmqpp::message msg;

  size_t header_sz;
  std::unique_ptr<const char[]> header =
      ClientCommandHeader(ClientProtocolCmd::kBrokerTerminate,
                          CommunicatorId::kClient, CommunicatorId::kBroker)
          .client_id(identity_)
          .Serialize(&header_sz);
  msg.push_front(header.get(), header_sz);
  msg.push_front(MDPC_CLIENT);

  logger_.Debug("Send termination request to broker...");
  socket_->send(msg);
}

}  // namespace bdm
