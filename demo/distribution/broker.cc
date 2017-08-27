#include "broker.h"

namespace bdm {

Broker::Broker(zmqpp::context* ctx, const std::string& endpoint,
               LoggingLevel level)
    : ctx_(ctx), endpoint_(endpoint), logger_("Broker", level) {
  this->socket_ = new zmqpp::socket(*ctx_, zmqpp::socket_type::router);
  this->hb_at_ =
      std::chrono::system_clock::now() + duration_ms_t(HEARTBEAT_INTERVAL);
}

Broker::~Broker() {
  if (socket_) {
    delete socket_;
  }
}

void Broker::Bind() {
  socket_->bind(endpoint_);
  logger_.Info("MDP broker/0.2.0 is active at ", endpoint_);
}

//  This method processes one READY, REPORT, HEARTBEAT or
//  DISCONNECT message sent to the broker by a worker

void Broker::HandleMessageWorker(const std::string& identity,
                                 std::unique_ptr<zmqpp::message> msg) {
  // Message format:
  // Frame 1:     WorkerCommandHeader class (serialized)
  // Frame 2..n:  Application frames
  assert(msg->parts() >= 1);  //  At least, command

  // Frame 1
  std::unique_ptr<WorkerCommandHeader> header =
      WorkerCommandHeader::Deserialize(msg->raw_data(0), msg->size(0));
  msg->pop_front();

  bool worker_ready = (workers_.find(identity) != workers_.end());
  WorkerEntry* worker = GetOrCreateWorker(identity);

  if (header->cmd_ == WorkerProtocolCmd::kReady) {
    if (worker_ready) {
      DeleteWorker(worker);
    } else {
      // Cross-check worker identity
      assert(header->worker_id_ == identity);

      // Monitor worker using heartbeats
      worker->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
      waiting_.insert(worker);

      logger_.Info("Worker ", identity, " created");
    }
  } else if (header->cmd_ == WorkerProtocolCmd::kReport) {
    if (worker_ready) {
      // Ignore MMI service for now
      // When MMI service is implemented, header->receiver_
      // will point to CommunicatorId::kBroker
      assert(header->receiver_ == CommunicatorId::kClient);

      //  Forward worker report message to appropriate client
      //
      // Message format:
      // Frame 1:     client_id (manually; ROUTER socket)
      // Frame 2:     "BDM/0.1C"
      // Frame 3:     ClientCommandHeader class (serialized)
      // Frame 4..n:  Application frames

      // Frame 3
      size_t c_header_sz;
      std::unique_ptr<const char[]> c_header =
          ClientCommandHeader(ClientProtocolCmd::kReport,
                              CommunicatorId::kSomeWorker,
                              CommunicatorId::kClient)
              .worker_id(worker->identity)
              .client_id(header->client_id_)
              .Serialize(&c_header_sz);
      msg->push_front(c_header.get(), c_header_sz);

      // Frame 2
      msg->push_front(MDPC_CLIENT);
      // Frame 1
      msg->push_front(header->client_id_);

      logger_.Debug("Broker sends message: ", *msg);
      socket_->send(*msg);

    } else {
      DeleteWorker(worker);
    }
  } else if (header->cmd_ == WorkerProtocolCmd::kHeartbeat) {
    if (worker_ready) {
      // Remove and reinsert worker to the waiting
      // queue after updating his expiration time
      waiting_.erase(waiting_.find(worker));
      worker->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
      waiting_.insert(worker);
    } else {
      DeleteWorker(worker);
    }
  } else if (header->cmd_ == WorkerProtocolCmd::kDisconnect) {
    // Delete worker without sending DISCONNECT message
    DeleteWorker(worker, false);
  } else {
    logger_.Error("Invalid input message", *msg);
  }
}

//  Process a request coming from a client.
void Broker::HandleMessageClient(const std::string& sender,
                                 std::unique_ptr<zmqpp::message> msg) {
  // Message format:
  // Frame 1:     ClientCommandHeader class (serialized)
  // Frame 2..n:  Application frames
  assert(msg->parts() >= 1);

  // Frame 1
  std::unique_ptr<ClientCommandHeader> header =
      ClientCommandHeader::Deserialize(msg->raw_data(0), msg->size(0));
  msg->pop_front();

  // Ignore MMI service for now
  // When MMI service is implemented, header->receiver_
  // will point to CommunicatorId::kBroker
  assert(header->receiver_ == CommunicatorId::kSomeWorker);

  if (workers_.find(header->worker_id_) == workers_.end()) {
    // Error: No such worker exist
    logger_.Warning("Invalid worker ", header->worker_id_, "! Droping message");

    // Message format:
    // Frame 1:     client_id (manually; ROUTER socket)
    // Frame 2:     "BDM/0.1C"
    // Frame 3:     ClientCommandHeader class (serialized)
    // Frame 4..n:  Application frames

    // Frame 3
    size_t header_sz;
    std::unique_ptr<const char[]> header =
        ClientCommandHeader(ClientProtocolCmd::kNak, CommunicatorId::kBroker,
                            CommunicatorId::kClient)
            .client_id(sender)
            .Serialize(&header_sz);
    msg->push_front(header.get(), header_sz);

    // Frame 2
    msg->push_front(MDPC_CLIENT);
    // Frame 1
    msg->push_front(sender);

    logger_.Debug("Sending NAK reply to client: ", *msg);

    // send NAK to client
    socket_->send(*msg);
  } else {
    WorkerEntry* worker = workers_[header->worker_id_];

    // Forward the pending messages to the worker
    worker->requests.push_back(msg->copy());

    while (!worker->requests.empty()) {
      zmqpp::message& pending = worker->requests.front();

      // TODO(kkanellis): fix client (what if client is different?)
      worker->Send(socket_, WorkerProtocolCmd::kRequest, &pending, sender);
      worker->requests.pop_front();
    }
  }
}

//  The purge method deletes any idle workers that haven't pinged us in a
//  while. We hold workers from oldest to most recent, so we can stop
//  scanning whenever we find a live worker. This means we'll mainly stop
//  at the first worker, which is essential when we have large numbers of
//  workers (since we call this method in our critical path)

void Broker::Purge() {
  while (!waiting_.empty()) {
    WorkerEntry* worker = *waiting_.begin();
    if (std::chrono::system_clock::now() < worker->expiry) {
      break;
    }

    DeleteWorker(worker, false);
  }
}

//  Lazy constructor that locates a worker by identity, or creates a new
//  worker if there is no worker already with that identity.

WorkerEntry* Broker::GetOrCreateWorker(const std::string& identity) {
  assert(!identity.empty());

  if (workers_.find(identity) == workers_.end()) {
    // Create worker and add him to workers
    WorkerEntry* worker = new WorkerEntry(identity, logger_);
    workers_[identity] = worker;

    logger_.Info("Registering new worker: ", identity);
  }
  return workers_[identity];
}

void Broker::DeleteWorker(WorkerEntry* worker, bool disconnect /* = true */) {
  if (disconnect) {
    worker->Send(socket_, WorkerProtocolCmd::kDisconnect);
  }

  // Remove from waiting & workers list
  waiting_.erase(worker);
  workers_.erase(worker->identity);
  delete worker;

  logger_.Info("Delete expired worker ", worker->identity);
}

void Broker::Run() {
  Bind();

  zmqpp::poller poller;
  poller.add(*socket_, zmqpp::poller::poll_in);

  while (true) {
    // Wait till heartbeat duration
    poller.poll(HEARTBEAT_INTERVAL.count());

    if (poller.events(*socket_) & zmqpp::poller::poll_in) {
      auto msg = std::make_unique<zmqpp::message>();
      if (!socket_->receive(*msg)) {
        break;  // Interrupted
      }

      // Message format received (expected)
      // Frame 1:     sender id (auto; DEALER socket)
      // Frame 2:     "BDM/0.1C" || "BDM/0.1W"
      // Frame 3:     *CommandHeader class (serialized)
      // Frame 4..n:  Application frames
      assert(msg->parts() >= 3);

      logger_.Debug("Broker received message: ", *msg);

      // Frame 1
      std::string sender = msg->get(0);
      msg->pop_front();

      // Frame 2
      std::string protocol = msg->get(0);
      msg->pop_front();

      if (protocol == MDPC_CLIENT) {
        HandleMessageClient(sender, std::move(msg));
      } else if (protocol == MDPW_WORKER) {
        HandleMessageWorker(sender, std::move(msg));
      } else {
        logger_.Error("Invalid message: ", *msg);
      }
    }

    //  Disconnect and delete any expired workers
    //  Send heartbeats to idle workers if needed
    if (std::chrono::system_clock::now() > hb_at_) {
      Purge();
      for (auto worker : waiting_) {
        worker->Send(socket_, WorkerProtocolCmd::kHeartbeat);
      }
      hb_at_ = std::chrono::system_clock::now() + HEARTBEAT_INTERVAL;
    }
  }
}
}  // namespace bdm
