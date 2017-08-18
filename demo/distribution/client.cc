#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include "client.h"
#include "common.h"

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

void Client::Send(const std::string& identity, zmqpp::message& msg) {
  //  Prefix request with protocol frames
  //  Frame 1: empty frame (delimiter)
  //  Frame 2: "MDPCxy" (six bytes, MDP/Client x.y)
  //  Frame 3: Worker identity (printable string)
  msg.push_front(identity);
  msg.push_front(MDPC_CLIENT);
  msg.push_front("");

  if (verbose) {
    std::cout << "I: send request to '" << identity << "' identity: " << msg
              << std::endl;
  }

  sock->send(msg);
}

bool Client::Recv(std::string* command_out, std::string* identity_out,
                  zmqpp::message& msg) {
  if (!sock->receive(msg)) {
    return false;
  }

  if (verbose) {
    std::cout << "I: received reply: " << msg << std::endl;
  }

  //  Message format:
  //  Frame 1: empty frame (delimiter)
  //  Frame 2: "MDPCxy" (six bytes, MDP/Client x.y)
  //  Frame 3: REPORT|NAK
  //  Frame 4: Worker identity (printable string)
  //  Frame 5..n: Application frames

  //  We would handle malformed replies better in real code
  assert(msg.parts() >= 5);

  std::string empty, header, command, identity;

  msg.get(empty, 0);
  msg.pop_front();
  assert(empty == "");

  msg.get(header, 0);
  msg.pop_front();
  assert(header == MDPC_CLIENT);

  msg.get(command, 0);
  msg.pop_front();
  assert(command == MDPC_REPORT || command == MDPC_NAK);

  if (command_out != nullptr) {
    *command_out = command;
  }

  msg.get(identity, 0);
  msg.pop_front();

  if (identity_out != nullptr) {
    *identity_out = identity;
  }

  // Success only when not received NAK
  return (command != MDPC_NAK);
}
}  // namespace bdm
