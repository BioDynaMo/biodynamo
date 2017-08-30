#include "dist_worker_api.h"

namespace bdm {

DistWorkerAPI::DistWorkerAPI(zmqpp::context* ctx, const std::string identity,
                             LoggingLevel level)
    : comms_(),
      wait_timeout_(2000),
      zctx_interrupted_(false),
      logger_("WAPI_[" + identity + "]", level) {
  info_.ctx_ = ctx;
  info_.identity_ = identity;
  info_.logging_level_ = level;
}

DistWorkerAPI::~DistWorkerAPI() {}

bool DistWorkerAPI::Start() {
  // Create (and bind) parent socket
  parent_pipe_ =
      std::make_unique<zmqpp::socket>(*(info_.ctx_), zmqpp::socket_type::pair);
  endpoint_ = "inproc://W_API_" + info_.identity_;

  try {
    parent_pipe_->bind(endpoint_);
  } catch (const zmqpp::zmq_internal_exception&) {
    logger_.Error("Endpoint '", info_.identity_, "' already taken");
    return false;
  }

  // Create child socket
  child_pipe_ =
      std::make_unique<zmqpp::socket>(*(info_.ctx_), zmqpp::socket_type::pair);
  child_pipe_->connect(endpoint_);

  // Create background thread
  thread_ = std::make_unique<std::thread>(
      std::bind(&DistWorkerAPI::HandleNetwork, this));
  logger_.Info("Started thread with id ", thread_->get_id());

  auto sig = parent_pipe_->wait();
  assert(sig == zmqpp::signal::ok || sig == zmqpp::signal::ko);
  return (sig == zmqpp::signal::ok);
}

void DistWorkerAPI::AddBrokerCommunicator(const std::string& endpoint) {
  auto& comm = comms_[ToUnderlying(CommunicatorId::kBroker)];
  assert(!comm);

  comm = std::make_unique<BrokerCommunicator>(&info_, endpoint);
}

void DistWorkerAPI::AddLeftNeighbourCommunicator(const std::string& endpoint) {
  auto& comm = comms_[ToUnderlying(CommunicatorId::kLeftNeighbour)];
  assert(!comm);

  comm = std::make_unique<WorkerCommunicator>(&info_, endpoint,
                                              CommunicatorId::kLeftNeighbour);
}

void DistWorkerAPI::AddRightNeighbourCommunicator(const std::string& endpoint) {
  auto& comm = comms_[ToUnderlying(CommunicatorId::kRightNeighbour)];
  assert(!comm);

  comm = std::make_unique<WorkerCommunicator>(&info_, endpoint,
                                              CommunicatorId::kRightNeighbour);
}

void DistWorkerAPI::SendDebugMessage(const std::string& value,
                                     CommunicatorId to) {
  auto msg = std::make_unique<zmqpp::message>();

  msg->push_front(value);

  // Create header
  size_t header_sz;
  std::unique_ptr<const char[]> header =
      AppMessageHeader(AppProtocolCmd::kDebug).Serialize(&header_sz);
  msg->push_front(header.get(), header_sz);

  SendRawMessage(std::move(msg), to);
}

bool DistWorkerAPI::ReceiveDebugMessage(std::string* value,
                                        CommunicatorId from) {
  // Wait for message
  std::unique_ptr<zmqpp::message> msg;
  std::unique_ptr<AppMessageHeader> header;
  if (!WaitForMessage(&msg, from, &header, AppProtocolCmd::kDebug)) {
    return false;
  }

  // Validate message type
  assert(msg->parts() == 1);
  assert(header->cmd_ == AppProtocolCmd::kDebug);

  // Extract value
  msg->get(*value, 0);
  return true;
}

bool DistWorkerAPI::ReceiveDebugMessageFromAny(std::string* value,
                                               CommunicatorId* from) {
  // Wait for message
  std::unique_ptr<zmqpp::message> msg;
  std::unique_ptr<AppMessageHeader> header;
  if (!WaitForMessageFromAny(&msg, from, &header, AppProtocolCmd::kDebug)) {
    return false;
  }

  // Validate message type
  assert(msg->parts() == 1);
  assert(header->cmd_ == AppProtocolCmd::kDebug);

  // Extract value
  msg->get(*value, 0);
  return true;
}

void DistWorkerAPI::SendRawMessage(std::unique_ptr<zmqpp::message> msg,
                                   CommunicatorId to) {
  // Queue for delivery
  std::unique_lock<std::mutex> lk(mq_net_deliver_mtx_);
  mq_net_deliver_.push(std::make_pair(std::move(msg), to));
  lk.unlock();

  // Notify the communication thread
  parent_pipe_->send(zmqpp::signal::test);
}

// Receive a message from specific communicator
bool DistWorkerAPI::WaitForMessage(
    std::unique_ptr<zmqpp::message>* msg, CommunicatorId from,
    std::unique_ptr<AppMessageHeader>* header /* = nullptr */,
    AppProtocolCmd cmd /* = AppProtocolCmd::kInvalid */) {
  assert(from != CommunicatorId::kUndefined);

  auto& queue = app_messages_[ToUnderlying(from)];
  auto predicate = [&queue, &cmd] {
    return (!queue.empty() && (cmd == AppProtocolCmd::kInvalid ||
                               queue.back().second->cmd_ == cmd));
  };

  std::unique_lock<std::mutex> lk(app_messages_mtx_);
  if (!predicate()) {
    // Wait for message from the network
    // Predicate is used to avoid spurious wakeups and to check the command
    if (!app_messages_cv_.wait_for(lk, wait_timeout_, predicate)) {
      // No proper message received
      return false;
    }
  }

  // No race condition here! We aquire the lock the moment we find
  // a suitable message, which is always located at the end of the deque

  // Message available
  *msg = std::move(queue.back().first);
  if (header != nullptr) {
    *header = std::move(queue.back().second);
  }
  queue.pop_back();
  return true;
}

bool DistWorkerAPI::WaitForMessageFromAny(
    std::unique_ptr<zmqpp::message>* msg, CommunicatorId* from,
    std::unique_ptr<AppMessageHeader>* header /* = nullptr */,
    AppProtocolCmd cmd /* = AppProtocolCmd::kInvalid */) {
  auto predicate = [this, &cmd] {
    return std::any_of(
        app_messages_.begin(), app_messages_.end(), [&cmd](auto& queue) {
          return (!queue.empty() && (cmd == AppProtocolCmd::kInvalid ||
                                     queue.back().second->cmd_ == cmd));
        });
  };

  std::unique_lock<std::mutex> lk(app_messages_mtx_);
  if (!predicate()) {
    // Wait for message from the network
    // Predicate is used to avoid spurious wakeups and to check the command
    if (!app_messages_cv_.wait_for(lk, wait_timeout_, predicate)) {
      // No proper message received
      return false;
    }
  }

  // No race condition here! We aquire the lock the moment we find
  // a suitable message, which is always located at the end of the deque

  // Message available
  std::uint8_t comm_id;
  for (auto& c : app_messages_) {
    if (!c.empty()) {
      auto& pair = c.back();
      *msg = std::move(pair.first);
      *from = static_cast<CommunicatorId>(comm_id);

      if (header != nullptr) {
        *header = std::move(pair.second);
      }
      c.pop_front();
      return true;
    }
    ++comm_id;
  }
  return false;
}

bool DistWorkerAPI::Stop(bool wait /* = true */, bool force /* = false */) {
  if (!parent_pipe_) {
    return false;
  }

  if (!parent_pipe_->send(zmqpp::signal::stop, true)) {
    // Cannot deliver signal. Maybe force kill?
    // TODO(kkanellis): handle force kill
    return false;
  }

  auto sig = parent_pipe_->wait();
  assert(sig == zmqpp::signal::ok || sig == zmqpp::signal::ko);
  if (sig == zmqpp::signal::ok) {
    thread_->join();

    parent_pipe_->close();
  }

  return (sig == zmqpp::signal::ok);
}

void DistWorkerAPI::HandleNetwork() {
  try {
    ForEachValidCommunicator(
        [](std::unique_ptr<Communicator>& comm) { comm->Connect(); });
  } catch (...) {
    eptr_ = std::current_exception();
    child_pipe_->send(zmqpp::signal::ko);

    logger_.Error("Exception thrown!");
    goto cleanup;
  }
  child_pipe_->send(zmqpp::signal::ok);

  // Add app_sock to reactor
  info_.reactor_.add(*child_pipe_,
                     std::bind(&DistWorkerAPI::HandleAppMessage, this));

  logger_.Info("Listening to network...");
  while (!zctx_interrupted_) {
    if (!info_.reactor_.poll(HEARTBEAT_INTERVAL.count())) {
      ForEachValidCommunicator(
          [](std::unique_ptr<Communicator>& comm) { comm->ReactorTimedOut(); });

      // Check if interrupted
      if (EINTR == zmq_errno()) {
        logger_.Warning("Interrupted...");
        zctx_interrupted_ = true;
        continue;
      }
    }

    // Handle pending messages from network
    if (!info_.mq_app_deliver_.empty()) {
      HandleNetworkMessages();
    }

    ForEachValidCommunicator([](std::unique_ptr<Communicator>& comm) {
      comm->ReactorServedRequests();
    });
  }

cleanup:
  Cleanup();
  logger_.Info("Terminated!");
}

void DistWorkerAPI::HandleAppMessage() {
  // The message must have the following format
  // Frame 1:    communicator identifier (where to send message?)
  // Frame 2:    recipient id
  // Frame 3..n: application frame(s)

  auto sig = zmqpp::signal();
  if (!child_pipe_->receive(sig) || sig == zmqpp::signal::stop) {
    // Interrupted
    zctx_interrupted_ = true;
    return;
  }
  // App wants to send a new message
  assert(sig == zmqpp::signal::test);

  std::unique_lock<std::mutex> lk(mq_net_deliver_mtx_);
  assert(!mq_net_deliver_.empty());

  // Find out where to forward the message
  auto& pair = mq_net_deliver_.front();
  auto& comm = GetValidCommunicator(pair.second);

  comm.HandleOutgoingMessage(std::move(pair.first));

  mq_net_deliver_.pop();
  // TODO(kkanellis): maybe release lock earlier?
}

void DistWorkerAPI::HandleNetworkMessages() {
  // Process all pending messages
  std::unique_lock<std::mutex> lk(app_messages_mtx_);
  while (!info_.mq_app_deliver_.empty()) {
    auto& pair = info_.mq_app_deliver_.front();

    // Verify that sender of the message exists
    auto comm_id = pair.second;
    assert(IsValidCommunicator(comm_id));

    // Deserialize & store application header
    std::unique_ptr<AppMessageHeader> header = AppMessageHeader::Deserialize(
        pair.first->raw_data(0), pair.first->size(0));
    pair.first->pop_front();

    // Push the message to the correct container & notify the app thread
    app_messages_[ToUnderlying(comm_id)].push_back(
        std::make_pair(std::move(pair.first), std::move(header)));
    info_.mq_app_deliver_.pop();
  }

  // Unlock & notify the app thread
  lk.unlock();
  app_messages_cv_.notify_one();
}

void DistWorkerAPI::Cleanup() {
  try {
    // Explicitly delete communicators
    ForEachValidCommunicator(
        [](std::unique_ptr<Communicator>& comm) { comm.reset(); });
    info_.reactor_.remove(*child_pipe_);

    // Everything cleaned!
    child_pipe_->send(zmqpp::signal::ok);
  } catch (...) {
    // Error occured; no gratefull exited
    child_pipe_->send(zmqpp::signal::ko);
  }

  child_pipe_->close();
}

void DistWorkerAPI::ForEachValidCommunicator(
    std::function<void(std::unique_ptr<Communicator>&)> f) {
  for (auto& comm : comms_) {
    if (comm) {
      f(comm);
    }
  }
}
}  // namespace bdm
