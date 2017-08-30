#include "broker.h"

namespace bdm {

Broker::Broker(zmqpp::context* ctx, const std::string& endpoint,
               LoggingLevel level)
    : ctx_(ctx),
      socket_(nullptr),
      endpoint_(endpoint),
      logger_("Broker", level) {
  socket_ = std::make_unique<zmqpp::socket>(*ctx_, zmqpp::socket_type::router);
  hb_at_ = std::chrono::system_clock::now() + duration_ms_t(HEARTBEAT_INTERVAL);
}

Broker::~Broker() {}

void Broker::Bind() {
  socket_->bind(endpoint_);
  logger_.Info("BDM broker/0.1 is active at ", endpoint_);
}

//  This method processes one READY, REPORT, HEARTBEAT or
//  DISCONNECT message sent to the broker by a worker
void Broker::HandleMessageWorker(const std::string& identity,
                                 std::unique_ptr<zmqpp::message> msg) {
  // Message format:
  // Frame 1:     WorkerMiddlewareMessageHeader class (serialized)
  // Frame 2..n:  Application frames
  assert(msg->parts() >= 1);  //  At least, command

  // Frame 1
  auto header =
      MessageUtil::PopFrontHeader<WorkerMiddlewareMessageHeader>(msg.get());

  bool worker_ready = (workers_.find(identity) != workers_.end());
  WorkerEntry* worker = GetOrCreateWorker(identity);

  if (header->cmd_ == WorkerProtocolCmd::kReady) {
    if (worker_ready) {
      DeleteWorker(worker);
    } else {
      // Cross-check worker identity
      assert(header->worker_id_ == identity);

      // Monitor worker using heartbeats
      worker->expiry_ = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
      waiting_.insert(worker);

      logger_.Info("Worker ", identity, " created");
    }
  } else if (header->cmd_ == WorkerProtocolCmd::kReport) {
    if (worker_ready) {
      // Ignore broker service for now
      // When MMI service is implemented, header->receiver_
      // will point to CommunicatorId::kBroker
      assert(header->receiver_ == CommunicatorId::kClient);

      //  Forward worker report message to appropriate client
      //
      // Message format:
      // Frame 1:     client_id (manually; ROUTER socket)
      // Frame 2:     "BDM/0.1C"
      // Frame 3:     ClientMiddlewareMessageHeader class (serialized)
      // Frame 4..n:  Application frames

      // Frame 3
      auto c_header = ClientMiddlewareMessageHeader(ClientProtocolCmd::kReport,
                                                    CommunicatorId::kSomeWorker,
                                                    CommunicatorId::kClient)
                          .worker_id(worker->identity_)
                          .client_id(header->client_id_);
      MessageUtil::PushFrontHeader(msg.get(), c_header);

      // Frame 2
      msg->push_front(PROTOCOL_CLIENT);
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
      worker->expiry_ = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
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
  // Frame 1:     ClientMiddlewareMessageHeader class (serialized)
  // Frame 2..n:  Application frames
  assert(msg->parts() >= 1);

  // Frame 1
  auto header =
      MessageUtil::PopFrontHeader<ClientMiddlewareMessageHeader>(msg.get());

  assert(sender == header->client_id_);

  bool worker_exists = (workers_.find(header->worker_id_) != workers_.end());

  if (header->receiver_ == CommunicatorId::kBroker) {
    // Handle broker service
    switch (header->cmd_) {
      case ClientProtocolCmd::kBrokerTerminate:
        req_termination_ = true;
        // Send ACK back to client
        ReplyToClient(ClientProtocolCmd::kAck, sender);
        break;
      case ClientProtocolCmd::kCheckWorker:
        // Reply with ACK if worker is connected; NAK otherwise
        {
          auto reply_cmd =
              worker_exists ? ClientProtocolCmd::kAck : ClientProtocolCmd::kNak;
          ReplyToClient(reply_cmd, sender, nullptr, header->worker_id_);
        }
        break;
      default:
        logger_.Error("Invalid command ", header->cmd_, " for broker!");
        assert(false);
    }

  } else if (header->receiver_ == CommunicatorId::kSomeWorker) {
    // Message destined for a worker

    if (!worker_exists) {
      logger_.Warning("Invalid worker ", header->worker_id_,
                      "! Droping message");

      // Send Nak to client
      ReplyToClient(ClientProtocolCmd::kNak, sender, nullptr,
                    header->worker_id_);
    } else {
      WorkerEntry* worker = workers_[header->worker_id_].get();

      // Forward the pending messages to the worker
      worker->Send(WorkerProtocolCmd::kRequest, std::move(msg), sender);
    }
  } else {
    logger_.Error("Invalid receiver: ", header->receiver_);
  }
}

void Broker::ReplyToClient(ClientProtocolCmd cmd, const std::string& client_id,
                           std::unique_ptr<zmqpp::message> msg /* = nullptr */,
                           const std::string& worker_id /* = "" */) {
  // Message format:
  // Frame 1:     client_id (manually; ROUTER socket)
  // Frame 2:     "BDM/0.1C"
  // Frame 3:     ClientMiddlewareMessageHeader class (serialized)
  // Frame 4..n:  Application frames

  if (!msg) {
    msg = std::make_unique<zmqpp::message>();
  }

  // Frame 3
  auto header = ClientMiddlewareMessageHeader(cmd, CommunicatorId::kBroker,
                                              CommunicatorId::kClient)
                    .client_id(client_id)
                    .worker_id(worker_id);
  MessageUtil::PushFrontHeader(msg.get(), header);

  // Frame 2
  msg->push_front(PROTOCOL_CLIENT);
  // Frame 1
  msg->push_front(client_id);

  // send NAK to client
  logger_.Debug("Sending ", cmd, " reply to client [", client_id, "]: ", *msg);
  socket_->send(*msg);
}

//  The purge method deletes any idle workers that haven't pinged us in a
//  while. We hold workers from oldest to most recent, so we can stop
//  scanning whenever we find a live worker. This means we'll mainly stop
//  at the first worker, which is essential when we have large numbers of
//  workers (since we call this method in our critical path)

void Broker::Purge() {
  while (!waiting_.empty()) {
    WorkerEntry* worker = *waiting_.begin();
    if (std::chrono::system_clock::now() < worker->expiry_) {
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
    workers_[identity] =
        std::make_unique<WorkerEntry>(socket_.get(), identity, logger_);

    logger_.Info("Registering new worker: ", identity);
  }
  return workers_[identity].get();
}

void Broker::DeleteWorker(WorkerEntry* worker, bool disconnect /* = true */) {
  if (disconnect) {
    worker->Send(WorkerProtocolCmd::kDisconnect);
  }
  logger_.Info("Delete expired worker ", worker->identity_);

  // Remove from waiting & workers list
  // Note: The deleting order is important
  waiting_.erase(worker);
  workers_.erase(worker->identity_);
}

void Broker::Run() {
  Bind();

  zmqpp::poller poller;
  poller.add(*socket_, zmqpp::poller::poll_in);

  // Run until someone requested termination and
  // there are no more connected workers
  while (!req_termination_ || !workers_.empty()) {
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
      // Frame 3:     *MiddlewareMessageHeader class (serialized)
      // Frame 4..n:  Application frames
      assert(msg->parts() >= 3);

      logger_.Debug("Broker received message: ", *msg);

      // Frame 1
      std::string sender = msg->get(0);
      msg->pop_front();

      // Frame 2
      std::string protocol = msg->get(0);
      msg->pop_front();

      if (protocol == PROTOCOL_CLIENT) {
        HandleMessageClient(sender, std::move(msg));
      } else if (protocol == PROTOCOL_WORKER) {
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
        worker->Send(WorkerProtocolCmd::kHeartbeat);
      }
      hb_at_ = std::chrono::system_clock::now() + HEARTBEAT_INTERVAL;
    }
  }
}
}  // namespace bdm
