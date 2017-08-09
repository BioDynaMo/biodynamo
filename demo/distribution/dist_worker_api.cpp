#include "dist_worker_api.hpp"

namespace mdp {

DistWorkerAPI::DistWorkerAPI(zmqpp::context *ctx, const std::string identity, bool verbose) {
    info_ = new DistSharedInfo();

    info_->reactor_ = new zmqpp::reactor();
    info_->ctx_ = ctx;
    info_->pending_ = new std::vector< std::unique_ptr<zmqpp::message> >();
    info_->identity_ = identity;
    info_->verbose_ = verbose;
}

DistWorkerAPI::~DistWorkerAPI () {
    delete thread_;
}

void DistWorkerAPI::StartThread(const std::string& app_endpoint) {
    this->info_->app_endpoint_ = app_endpoint;

    thread_ = new std::thread(
        std::bind(&DistWorkerAPI::HandleNetwork, this)
    );
    std::cout << "I: started thread with id " << thread_->get_id() << std::endl;
}

void DistWorkerAPI::SetBrokerEndpoint(const std::string& endpoint) {
    this->broker_endpoint_ = endpoint;
}

void DistWorkerAPI::SetLeftNeighbourEndpoint(const std::string& endpoint) {
    this->lworker_endpoint_ = endpoint;
}

void DistWorkerAPI::SetRightNeighbourEndpoint(const std::string& endpoint) {
    this->rworker_endpoint_ = endpoint;
}

void DistWorkerAPI::WaitForTermination() {
   thread_->join();
   thread_ = nullptr;
}

void DistWorkerAPI::HandleNetwork() {
    // Create local/application socket
    info_->app_socket_ = new zmqpp::socket( *(info_->ctx_), zmqpp::socket_type::pair);
    info_->app_socket_->connect(info_->app_endpoint_);

    // Add app_sock to reactor
    info_->reactor_->add( *(info_->app_socket_), std::bind(&DistWorkerAPI::HandleAppMessage, this));

    // Create broker communicator
    if (!broker_endpoint_.empty()) {
        broker_comm_ = new BrokerCommunicator(info_, broker_endpoint_);
    }

    // Create left communicator
    if (!lworker_endpoint_.empty()) {
        lworker_comm_ = new WorkerCommunicator(info_, lworker_endpoint_, true);
    }

    // Create right communicator
    if (!rworker_endpoint_.empty()) {
        rworker_comm_ = new WorkerCommunicator(info_, rworker_endpoint_, false);
    }

    while ( !info_->zctx_interrupted_ ) {
        if ( !info_->reactor_->poll ( HEARTBEAT_INTERVAL.count() ) ) {
            if (broker_comm_) {
                broker_comm_->RequestTimedOut();
            }
        }

        if (!info_->pending_->empty()) {
            // Handle pending messages from network
            HandleNetworkMessages();
        }

        if (broker_comm_) {
            broker_comm_->RequestCompleted();
        }
    }

    Terminate();
    std::cout << "I: Terminated!" << std::endl;
}

void DistWorkerAPI::HandleAppMessage () {
    // The message must have the following format
    // Frame 1: communicator identifier (where to send message?)
    // Frame 2: recipient id
    // Frame 3: application frame(s)

    zmqpp::message msg;
    if ( !info_->app_socket_->receive(msg)
            || msg.is_signal() ) {
        // Interrupted
        info_->zctx_interrupted_ = true;
        return;
    }

    std::uint8_t comm_id;
    std::string recipient;

    // Find out where to send the message
    msg.get(comm_id, 0);
    msg.pop_front();

    if (comm_id == BROKER_COMM) {
        assert(broker_comm_);

        // Check recipient address
        msg.get(recipient, 1);
        assert( !recipient.empty() );

        broker_comm_->HandleOutgoingMessage(msg);
    }
    else if (comm_id == LEFT_NEIGHBOUR_COMM) {
        assert(lworker_comm_);
        lworker_comm_->HandleOutgoingMessage(msg);
    }
    else if (comm_id == RIGHT_NEIGHBOUR_COMM) {
        assert(rworker_comm_);
        rworker_comm_->HandleOutgoingMessage(msg);
    }
    else {
        std::cout << "E: Invalid communicator id: " << comm_id << std::endl;
    }
}

void DistWorkerAPI::HandleNetworkMessages() {
    std::uint8_t comm_id;
    for (auto& msg_p : *(info_->pending_)) {
        // Retrieve the source of the message
        msg_p->get(comm_id, 0);

        if (comm_id == BROKER_COMM) {
            assert(broker_comm_);
            info_->app_socket_->send(*msg_p);
        }
        else if (comm_id == LEFT_NEIGHBOUR_COMM) {
            assert(lworker_comm_);
            info_->app_socket_->send(*msg_p);
        }
        else if (comm_id == RIGHT_NEIGHBOUR_COMM) {
            assert(rworker_comm_);
            info_->app_socket_->send(*msg_p);
        }
        else {
            std::cout << "E: Invalid communicator id: " << comm_id << std::endl;
        }

    }
    info_->pending_->clear();
}

void DistWorkerAPI::Terminate() {
    if (broker_comm_) {
        delete broker_comm_;
    }
    if (lworker_comm_) {
        delete lworker_comm_;
    }
    if (rworker_comm_) {
        delete rworker_comm_;
    }

    info_->reactor_->remove( *(info_->app_socket_) );
    info_->app_socket_->close();
    delete info_->app_socket_;

    delete info_->reactor_;
}


}
