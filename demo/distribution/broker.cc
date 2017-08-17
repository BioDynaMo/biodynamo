#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include "broker.h"
#include "worker_entry.h"

namespace bdm {

Broker::Broker (zmqpp::context *ctx, const std::string& endpoint, const bool verbose_)
    : ctx_ (ctx)
    , endpoint_(endpoint)
    , verbose_ (verbose_) {

    this->socket_ = new zmqpp::socket(*ctx_, zmqpp::socket_type::router);
    this->hb_at_ = std::chrono::system_clock::now() + \
                    duration_ms_t(HEARTBEAT_INTERVAL);
}

Broker::~Broker() {
    if (socket_) {
        delete socket_;
    }
}

void
Broker::Bind() {
    socket_->bind(endpoint_);
    std::cout << "I: MDP broker/0.2.0 is active at " << endpoint_ << std::endl;
}

//  This method processes one READY, REPORT, HEARTBEAT or
//  DISCONNECT message sent to the broker by a worker

void
Broker::HandleMessageWorker(const std::string& identity, zmqpp::message& msg) {
    assert(msg.parts() >= 1); //  At least, command

    std::string command;
    msg.get(command, 0);
    msg.pop_front();

    bool worker_ready = (workers_.find(identity) != workers_.end());
    WorkerEntry* worker = GetOrCreateWorker(identity);

    if (command == MDPW_READY) {
        if (worker_ready) {
            DeleteWorker(worker);
        }
        // else if ( MMI stuff )
        else {
            std::string sent_id;
            msg.get(sent_id, 0);
            msg.pop_front();

            assert(sent_id == identity);

            // monitor worker using heartbeats
            worker->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
            waiting_.insert(worker);

            std::cout << "I: worker " << identity << " created" << std::endl;
        }
    }
    else if (command == MDPW_REPORT) {
        if (worker_ready) {
            //  Remove & save client return envelope and insert the
            //  protocol header and service name, then rewrap envelope.
            std::string client;
            msg.get(client, 0);
            msg.pop_front();

            msg.push_front (worker->identity);
            msg.push_front (MDPC_REPORT);
            msg.push_front (MDPC_CLIENT);
            msg.push_front ("");
            msg.push_front (client);

            socket_->send(msg);

        }
        else {
            DeleteWorker(worker);
        }
    }
    else if (command == MDPW_HEARTBEAT) {
        if (worker_ready) {
            // Remove and reinsert worker to the waiting_
            // queue after updating his expiration time
            waiting_.erase( waiting_.find(worker) );
            worker->expiry = std::chrono::system_clock::now() + HEARTBEAT_EXPIRY;
            waiting_.insert(worker);
        }
        else {
            DeleteWorker(worker);
        }
    }
    else if (command == MDPW_DISCONNECT) {
        DeleteWorker(worker, false);
    }
    else {
        std::cout << "E: invalid input message" << msg << std::endl;
    }
}

//  Process a request coming from a client. We implement MMI requests
//  directly here (at present, we implement only the mmi.service request)

void
Broker::HandleMessageClient(const std::string& sender, zmqpp::message& msg) {
    assert (msg.parts() >= 2);  // service/identity name + body

    // Get worker
    std::string worker_identity;
    msg.get(worker_identity, 0);

    if ( workers_.find(worker_identity) == workers_.end() ) {
        // no such worker exist
        std::cout << "E: invalid worker " << worker_identity \
            << ". Droping message..." << std::endl;

        // send NAK to client
        msg.push_front (MDPC_NAK);
        msg.push_front (MDPC_CLIENT);
        msg.push_front ("");
        msg.push_front (sender);

        socket_->send(msg);
        return;
    }

    WorkerEntry *worker = workers_[worker_identity];

    // NOTE NOTE NOTE
    // ignore MMI service for now

    // Forward the pending messages to the worker
    msg.push_front("");
    msg.push_front(sender);
    worker->requests.push_back(msg.copy());

    while ( !worker->requests.empty() ) {
        zmqpp::message& pending = worker->requests.front();
        worker->Send(socket_, MDPW_REQUEST, &pending, verbose_);
        worker->requests.pop_front();
    }

}

//  The purge method deletes any idle workers that haven't pinged us in a
//  while. We hold workers from oldest to most recent, so we can stop
//  scanning whenever we find a live worker. This means we'll mainly stop
//  at the first worker, which is essential when we have large numbers of
//  workers (since we call this method in our critical path)

void
Broker::Purge () {
    while (!waiting_.empty()) {
        WorkerEntry* worker = *waiting_.begin();
        if (std::chrono::system_clock::now() <  worker->expiry ) {
            break;
        }

        DeleteWorker(worker, false);
    }
}


//  Lazy constructor that locates a worker by identity, or creates a new
//  worker if there is no worker already with that identity.

WorkerEntry*
Broker::GetOrCreateWorker(const std::string& identity) {
    assert( !identity.empty() );

    if ( workers_.find(identity) == workers_.end() ) {
        // Create worker and add him to workers
        WorkerEntry *worker = new WorkerEntry(identity);
        workers_[identity] = worker;

        std::cout << "I: registering new worker: " << identity << std::endl;
    }
    return workers_[identity];
}

void
Broker::DeleteWorker(WorkerEntry *worker, bool disconnect /* = true */) {
    if (disconnect) {
        worker->Send(socket_, MDPW_DISCONNECT, nullptr, verbose_);
    }

    // Remove from waiting & workers list
    waiting_.erase (worker);
    workers_.erase (worker->identity);
    delete worker;

    std::cout << "I: delete expired worker " << worker->identity << std::endl;
}

void
Broker::Run() {
    Bind();

    zmqpp::poller poller;
    poller.add(*socket_, zmqpp::poller::poll_in);

    zmqpp::message msg;
    while (true) {
        // Wait till heartbeat duration
        poller.poll(HEARTBEAT_INTERVAL.count());

        if (poller.events(*socket_) & zmqpp::poller::poll_in) {
            if ( !socket_->receive(msg) ) {
                break; // Interrupted
            }

            if (verbose_) {
                std::cout << "I: received message: " << msg << std::endl;
            }

            std::string sender, empty, header;

            msg.get(sender, 0);
            msg.pop_front();

            msg.get(empty, 0);
            msg.pop_front();

            msg.get(header, 0);
            msg.pop_front();

            if (header == MDPC_CLIENT) {
                HandleMessageClient(sender, msg);
            }
            else if (header == MDPW_WORKER) {
                HandleMessageWorker(sender, msg);
            }
            else {
                std::cout << "E: invalid message: " << msg << std::endl;
            }
        }

        //  Disconnect and delete any expired workers
        //  Send heartbeats to idle workers if needed
        if (std::chrono::system_clock::now() > hb_at_) {
            Purge();
            for (auto worker : waiting_) {
                worker->Send(socket_, MDPW_HEARTBEAT, nullptr, verbose_);
            }
            hb_at_ = std::chrono::system_clock::now() + HEARTBEAT_INTERVAL;
        }

    }

}


}
