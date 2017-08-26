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
  msg->push_front(ToUnderlying(to));
  parent_pipe_->send(*msg);
}

bool DistWorkerAPI::ReceiveMessage(std::unique_ptr<zmqpp::message>* msg,
                                   CommunicatorId from, duration_ms_t timeout) {
  assert(from > CommunicatorId::kUndefined);

  // Read all pending messages from the pipe
  ReceiveAllMessages();

  auto& queue = msg_queues_[ToUnderlying(from)];
  if (queue.empty()) {
    // Wait for message until timeout
    // NOTE: we use the predicate to avoid spurious wakeups
    std::unique_lock<std::mutex> lk(msgs_cv_m);
    msgs_cv_.wait_for(lk, timeout, [this, &queue] {
      ReceiveAllMessages();
      return !queue.empty();
    });
  }

  ReceiveAllMessages();
  bool empty_queue = queue.empty();
  if (!empty_queue) {
    *msg = std::move(*(queue.begin()));
    queue.pop_front();
  }

  return !empty_queue;
}

bool DistWorkerAPI::ReceiveMessage(std::unique_ptr<zmqpp::message>* msg,
                                   duration_ms_t timeout) {
  // Read all pending messages from the pipe
  ReceiveAllMessages();

  auto predicate = [](auto& queue) { return !queue.empty(); };
  if (!std::any_of(msg_queues_.begin(), msg_queues_.end(), predicate)) {
    // Wait for message until timeout
    // NOTE: we use the predicate to avoid spurious wakeup
    std::unique_lock<std::mutex> lk(msgs_cv_m);
    msgs_cv_.wait_for(lk, timeout, [this, &predicate] {
      ReceiveAllMessages();
      return std::any_of(msg_queues_.begin(), msg_queues_.end(), predicate);
    });
  }

  // Return message from anyone
  ReceiveAllMessages();
  for (auto& c : msg_queues_) {
    if (!c.empty()) {
      *msg = std::move(*(c.begin()));
      c.pop_front();
      return true;
    }
  }

  return false;
}

bool DistWorkerAPI::IsConnected(const CommunicatorId comm) {
  auto id = ToUnderlying(comm);
  if (!IsValidCommunicator(id)) {
    return false;
  }

  return GetValidCommunicator(id).IsConnected();
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

    if (!info_.pending_.empty()) {
      // Handle pending messages from network
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

  auto msg = std::make_unique<zmqpp::message>();
  if (!child_pipe_->receive(*msg) || msg->is_signal()) {
    // Interrupted
    info_.zctx_interrupted_ = true;
    return;
  }

  // TODO(kkanellis): w->w don't need receipient id
  // assert(msg->parts() >= 3);

  // Find out where to forward the message
  std::uint8_t comm_id;
  msg->get(comm_id, 0);
  msg->pop_front();

  auto& comm = GetValidCommunicator(comm_id);
  comm.HandleOutgoingMessage(std::move(msg));
}

void DistWorkerAPI::HandleNetworkMessages() {
  std::uint8_t comm_id;
  for (auto& msg_p : info_.pending_) {
    // Verify that sender exists
    msg_p->get(comm_id, 0);
    assert(IsValidCommunicator(comm_id));

    child_pipe_->send(*msg_p);
    msgs_cv_.notify_one();
  }
  info_.pending_.clear();
}

void DistWorkerAPI::ReceiveAllMessages() {
  std::uint8_t comm_id;

  auto msg = std::make_unique<zmqpp::message>();
  while (parent_pipe_->receive(*msg, true)) {
    // Push to the proper container
    msg->get(comm_id, 0);
    msg->pop_front();
    assert(IsValidCommunicator(comm_id));

    msg_queues_[comm_id].push_back(std::move(msg));

    // Next message?
    msg = std::make_unique<zmqpp::message>();
  }
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

bool DistWorkerAPI::IsValidCommunicator(std::uint8_t comm_id) {
  if (comm_id == 0 || comm_id >= comms_.size()) {
    logger_.Error("Invalid communicator id: ", comm_id);
    assert(false);
  }
  return (comms_[comm_id] != nullptr);
}

Communicator& DistWorkerAPI::GetValidCommunicator(std::uint8_t comm_id) {
  assert(IsValidCommunicator(comm_id));
  return *comms_[comm_id];
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
