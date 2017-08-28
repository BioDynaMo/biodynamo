#include "dist_worker_api.h"

namespace bdm {

DistWorkerAPI::DistWorkerAPI(zmqpp::context* ctx, const std::string identity,
                             LoggingLevel level)
    : comms_(), logger_("WAPI_[" + identity + "]", level) {
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

void DistWorkerAPI::SendMessage(std::unique_ptr<zmqpp::message> msg,
                                CommunicatorId to) {
  // TODO(kkanellis): create application header

  // Create header
  std::unique_lock<std::mutex> lk(mq_net_deliver_mtx_);
  mq_net_deliver_.push(std::make_pair(std::move(msg), to));
  lk.unlock();

  // Notify the communication thread
  parent_pipe_->send(zmqpp::signal::test);
}

bool DistWorkerAPI::ReceiveMessage(std::unique_ptr<zmqpp::message>* msg,
                                   CommunicatorId from, duration_ms_t timeout) {
  assert(from > CommunicatorId::kUndefined);

  auto& queue = app_messages_[ToUnderlying(from)];
  if (queue.empty()) {
    // Wait for message until timeout
    // NOTE: we use the predicate to avoid spurious wakeups
    std::unique_lock<std::mutex> lk(app_messages_mtx_);
    app_messages_cv_.wait_for(lk, timeout,
                              [this, &queue] { return !queue.empty(); });
  }

  if (!queue.empty()) {
    *msg = std::move(*(queue.begin()));
    queue.pop_front();
    return true;
  }

  // No message available
  return false;
}

bool DistWorkerAPI::ReceiveMessage(std::unique_ptr<zmqpp::message>* msg,
                                   CommunicatorId* from,
                                   duration_ms_t timeout) {
  auto predicate = [](auto& queue) { return !queue.empty(); };
  if (!std::any_of(app_messages_.begin(), app_messages_.end(), predicate)) {
    // Wait for message until timeout
    // NOTE: we use the predicate to avoid spurious wakeup
    std::unique_lock<std::mutex> lk(app_messages_mtx_);
    app_messages_cv_.wait_for(lk, timeout, [this, &predicate] {
      return std::any_of(app_messages_.begin(), app_messages_.end(), predicate);
    });
  }

  // Return message from anyone
  std::uint8_t comm_id;
  for (auto& c : app_messages_) {
    if (!c.empty()) {
      *msg = std::move(*(c.begin()));
      c.pop_front();

      if (from != nullptr) {
        *from = static_cast<CommunicatorId>(comm_id);
      }

      return true;
    }

    ++comm_id;
  }

  // No message available
  return false;
}

bool DistWorkerAPI::IsConnected(const CommunicatorId comm) const {
  if (!IsValidCommunicator(comm)) {
    return false;
  }
  return GetValidCommunicator(comm).IsConnected();
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
  while (!info_.zctx_interrupted_) {
    if (!info_.reactor_.poll(HEARTBEAT_INTERVAL.count())) {
      ForEachValidCommunicator(
          [](std::unique_ptr<Communicator>& comm) { comm->ReactorTimedOut(); });
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
    info_.zctx_interrupted_ = true;
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

    // Push the message to the correct container & notify the app thread
    app_messages_[ToUnderlying(comm_id)].push_back(std::move(pair.first));
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

inline bool DistWorkerAPI::IsValidCommunicator(CommunicatorId comm_id) const {
  std::uint8_t id = ToUnderlying(comm_id);
  if (id == 0 || id >= comms_.size()) {
    logger_.Error("Invalid communicator id: ", id);
    assert(false);
  }
  return (comms_[id] != nullptr);
}

inline Communicator& DistWorkerAPI::GetValidCommunicator(
    CommunicatorId comm_id) const {
  assert(IsValidCommunicator(comm_id));
  return *comms_[ToUnderlying(comm_id)];
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
