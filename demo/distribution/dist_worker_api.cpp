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
    delete info_;
}

void DistWorkerAPI::StartThread(const std::string& app_endpoint) {
    this->info_->app_endpoint_ = app_endpoint;

    thread_ = new std::thread(
        std::bind(&DistWorkerAPI::HandleNetwork, this)
    );
    std::cout << "I: started thread with id " << thread_->get_id() << std::endl;
}


void DistWorkerAPI::HandleNetwork() {
    // Create local/application socket
    info_->app_socket_ = new zmqpp::socket( *(info_->ctx_), zmqpp::socket_type::pair);
    info_->app_socket_->connect(info_->app_endpoint_);
    
    // Add app_sock to reactor
    std::cout << info_->app_socket_ << std::endl;
    info_->reactor_->add( *(info_->app_socket_), std::bind(&DistWorkerAPI::HandleAppMessage, this));
    
    // Create broker communicator
    broker_comm_ = new BrokerCommunicator(info_, "tcp://localhost:5555");

    while ( !info_->zctx_interrupted_ ) {
        if ( !info_->reactor_->poll ( HEARTBEAT_INTERVAL.count() ) ) {
            broker_comm_->RequestTimedOut();
        }
        
        if (!info_->pending_->empty()) {
            // Handle pending messages from network
            HandleNetworkMessages();
        }

        broker_comm_->RequestCompleted();
    }
}

void DistWorkerAPI::HandleAppMessage () {
    // The message must have the following format
    // Frame 1: communicator identifier (where to send message?)
    // Frame 2: recipient id
    // Frame 3: application frame(s)

    zmqpp::message msg;
    if ( !info_->app_socket_->receive(msg) ) {
        // Interrupted
        info_->zctx_interrupted_ = true;
        return;
    }
        
    std::uint8_t comm_id;
    std::string recipient;

    // Check recipient address
    msg.get(recipient, 1);
    assert( !recipient.empty() );

    // Find out where to send the message
    msg.get(comm_id, 0);
    msg.pop_front();

    if (comm_id == BROKER_COMM) {
        broker_comm_->HandleOutgoingMessage(msg);
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
            info_->app_socket_->send(*msg_p);
        }
        else {
            std::cout << "E: Invalid communicator id: " << comm_id << std::endl;
        }

    }
    info_->pending_->clear();
}

}
