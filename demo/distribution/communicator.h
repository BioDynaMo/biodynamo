#ifndef DEMO_DISTRIBUTION_COMMUNICATOR_H_
#define DEMO_DISTRIBUTION_COMMUNICATOR_H_

#include <string>

#include "common.h"

namespace bdm {

/// @brief Abstract class that defines the interface for communicating with
///        a remote node, via network.
///
/// All the class methods are executed by the network thread.
///
class Communicator {
 public:
  Communicator(DistSharedInfo* info, std::string endpoint,
               CommunicatorId comm_id)
      : info_(info),
        socket_(nullptr),
        endpoint_(endpoint),
        comm_id_(comm_id),
        is_connected_(false) {}

  virtual ~Communicator() {}

  ///
  /// @brief Callback when the reactor times out
  ///
  virtual void ReactorTimedOut() {}

  ///
  /// @brief Callback when the reactor has just served a request (either from
  ///        the app or from the network)
  ///
  virtual void ReactorServedRequests() {}

  ///
  /// @brief Initiates the connection procedure to the remote node
  ///
  virtual void Connect() = 0;

  ///
  /// @brief Sends a message through the network to the remote node
  ///
  virtual void HandleOutgoingMessage(std::unique_ptr<zmqpp::message> msg) = 0;

  ///
  /// @brief Receive all available messages from the remote node
  ///
  virtual void HandleIncomingMessages() = 0;

  ///
  /// @brief Returns the #CommunicatorId that this communicator manages
  ///
  virtual CommunicatorId GetCommunicationId() {
    assert(comm_id_ != CommunicatorId::kUndefined);
    return comm_id_;
  }

  ///
  /// @brief Returns a pointer to this communicator zmq socket
  ///
  virtual zmqpp::socket* GetSocketPtr() { return socket_.get(); }

  ///
  /// @brief Returns \p true if the remote node is alive and responsible
  ///
  virtual bool IsConnected() { return is_connected_; }

  ///
  /// @brief Sets a zmqpp::socket option to a specific value
  ///
  /// @param[in]  option  zmq socket option to be set
  /// @param[in]  value   new value for \p option
  ///
  /// @tparam value type of \p option
  ///
  template <typename T>
  void SetSocketOption(const zmqpp::socket_option& option, const T& value) {
    socket_->set(option, value);
  }

  ///
  /// @brief Returns value of a zmqpp::socket option
  ///
  /// @param[in]  option  zmq socket option to be st
  ///
  /// @tparam value type of \p option
  ///
  /// @return  the current value of for option
  ///
  template <typename T>
  T GetSocketOption(const zmqpp::socket_option& option) {
    T value;
    GetSocketOption(option, &value);
    return value;
  }

  ///
  /// @brief Returns value of a zmqpp::socket option
  ///
  /// @param[in]  option  zmq socket option to be st
  /// @param[out] value   the current value of \p option
  ///
  /// @tparam value type of \p option
  ///
  template <typename T>
  void GetSocketOption(const zmqpp::socket_option& option, T* value) {
    socket_->get(option, *value);
  }

 protected:
  DistSharedInfo* info_;  //!< Pointer to shared variables needed by the API

  std::unique_ptr<zmqpp::socket> socket_;  //!< ZMQ socket object
  std::string endpoint_;                   //!< Local node endpoint

  CommunicatorId comm_id_;  //!< Local node #CommunicatorId
  bool is_connected_;       //!< \p True if remote node is alive and responsive
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_COMMUNICATOR_H_
