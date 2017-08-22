#include "client.h"

namespace bdm {

Client::Client(zmqpp::context* ctx, const std::string& broker, bool verbose) {
  assert(!broker.empty());

  this->ctx = ctx;
  this->broker = broker;
  this->verbose = verbose;
  this->timeout = duration_ms_t(2500);  // msecs

  ConnectToBroker();
}

Client::~Client() {
  sock->close();
  delete sock;
}

void Client::ConnectToBroker() {
  if (sock) {
    sock->close();
    delete sock;
  }

  sock = new zmqpp::socket(*ctx, zmqpp::socket_type::dealer);
  sock->set(zmqpp::socket_option::identity, "client");
  sock->connect(broker);

  std::cout << "I: connecting to broker at " << broker << std::endl;
}

void Client::SetTimeout(duration_ms_t timeout) { this->timeout = timeout; }

template <typename T>
void Client::SetSocketOption(zmqpp::socket_option option, const T& value) {
  sock->set(option, value);
}

template <typename T>
T Client::GetSocketOption(zmqpp::socket_option option) {
  T value;
  GetSocketOption(option, &value);
  return value;
}

template <typename T>
void Client::GetSocketOption(zmqpp::socket_option option, T* value) {
  sock->get(option, *value);
}

//  Here is the send method. It sends a request to the broker.
//  It takes ownership of the request message, and destroys it when sent.

void Client::Send(const std::string& identity,
                  std::unique_ptr<zmqpp::message> msg) {
  //  Message format:
  //  Frame 1:    "BDM/0.1C"
  //  Frame 2:    ClientCommandHeader class (serialized)
  //  Frame 3..n: Application frames

  // TODO(kkanellis): add identity to client
  std::unique_ptr<std::string> header =
      ClientCommandHeader(ClientProtocolCmd::kRequest, CommunicatorId::kClient,
                          CommunicatorId::kSomeWorker)
          .worker_id(identity)
          .ToString();

  // Frame 2
  msg->push_front(*header);
  // Frame 1
  msg->push_front(MDPC_CLIENT);

  if (verbose) {
    std::cout << "I: send request to '" << identity << "' identity: " << *msg
              << std::endl;
  }
  sock->send(*msg);
}

// TODO(kkanellis): change name of arguments
bool Client::Recv(std::unique_ptr<zmqpp::message>* msg_out,
                  ClientProtocolCmd* command_out /* = nullptr */,
                  std::string* identity_out /* = nullptr */) {
  auto msg = std::make_unique<zmqpp::message>();
  if (!sock->receive(*msg)) {
    return false;
  }

  if (verbose) {
    std::cout << "I: received reply: " << *msg << std::endl;
  }

  //  Message format:
  //  Frame 1:    "BDM/0.1C"
  //  Frame 2:    ClientCommandHeader class (serialized)
  //  Frame 3..n: Application frames
  assert(msg->parts() >= 2);

  // Read protocol version
  std::string protocol = msg->get(0);
  msg->pop_front();
  assert(protocol == MDPC_CLIENT);

  std::string* header_str = new std::string(msg->get(0));
  msg->pop_front();

  std::unique_ptr<ClientCommandHeader> header =
      ClientCommandHeader::FromString<ClientCommandHeader>(header_str);
  assert(header->cmd_ == ClientProtocolCmd::kReport ||
         header->cmd_ == ClientProtocolCmd::kNak);

  if (command_out != nullptr) {
    *command_out = header->cmd_;
  }

  if (identity_out != nullptr) {
    *identity_out = header->worker_id_;
  }

  // Success only when not received NA
  *msg_out = std::move(msg);
  return (header->cmd_ != ClientProtocolCmd::kNak);
}
}  // namespace bdm
