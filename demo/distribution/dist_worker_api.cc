#include "dist_worker_api.h"

namespace mdp {

DistWorkerAPI::DistWorkerAPI(zmqpp::context *ctx, const std::string identity, bool verbose) : comms_() {
    info_ = new DistSharedInfo();

    info_->reactor_ = new zmqpp::reactor();
    info_->ctx_ = ctx;
    info_->pending_ = new std::vector< std::unique_ptr<zmqpp::message> >();
    info_->identity_ = identity;
    info_->verbose_ = verbose;

    // Invalid communicators
    comms_.resize(
        ToUnderlying(CommunicatorId::kCount)
    );
}

DistWorkerAPI::~DistWorkerAPI () {
    delete thread_;
}

bool DistWorkerAPI::Start() {
    // Create (and bind) parent socket
    parent_pipe_ = new zmqpp::socket( *(info_->ctx_), zmqpp::socket_type::pair);
    endpoint_ = "inproc://W_API_" + info_->identity_;

    try {
        parent_pipe_->bind(endpoint_);
    }
    catch (const zmqpp::zmq_internal_exception& ) {
        std::cout << "E: Endpoint '" << info_->identity_ << "' already taken" << std::endl;
        return false;
    }

    // Create child socket
    child_pipe_ = new zmqpp::socket( *(info_->ctx_), zmqpp::socket_type::pair);
    child_pipe_->connect(endpoint_);

    // Create background thread
    thread_ = new std::thread(
        std::bind(&DistWorkerAPI::HandleNetwork, this)
    );
    std::cout << "I: started thread with id " << thread_->get_id() << std::endl;

    auto sig = parent_pipe_->wait();
    assert(sig == zmqpp::signal::ok || sig == zmqpp::signal::ko);
    return (sig == zmqpp::signal::ok);
}

void DistWorkerAPI::AddBrokerCommunicator(const std::string& endpoint) {
    auto& comm = comms_[ToUnderlying(CommunicatorId::kBroker)];
    assert( !comm );

    comm = std::unique_ptr<Communicator>(
        new BrokerCommunicator(info_, endpoint)
    );
}

void DistWorkerAPI::AddLeftNeighbourCommunicator(const std::string& endpoint) {
    auto& comm = comms_[ToUnderlying(CommunicatorId::kLeftNeighbour)];
    assert( !comm );

    comm = std::unique_ptr<Communicator>(
        new WorkerCommunicator(info_, endpoint, CommunicatorId::kLeftNeighbour)
    );
}

void DistWorkerAPI::AddRightNeighbourCommunicator(const std::string& endpoint) {
    auto& comm = comms_[ToUnderlying(CommunicatorId::kRightNeighbour)];
    assert( !comm );

    comm = std::unique_ptr<Communicator>(
        new WorkerCommunicator(info_, endpoint, CommunicatorId::kRightNeighbour)
    );
}

void DistWorkerAPI::SendMessage(zmqpp::message& msg) {
    parent_pipe_->send(msg);
}


void DistWorkerAPI::ReceiveMessage(zmqpp::message *msg) {
    parent_pipe_->receive( *msg);
}

bool DistWorkerAPI::Stop(bool wait /* = true */, bool force /* = false */) {
    if (!parent_pipe_) {
        return false;
    }

    if ( !parent_pipe_->send(zmqpp::signal::stop, true) ) {
        // Cannot deliver signal. Maybe force kill?
        // TODO: handle force kill
        return false;
    }

    auto sig = parent_pipe_->wait();
    assert(sig == zmqpp::signal::ok || sig == zmqpp::signal::ko);
    if (sig == zmqpp::signal::ok) {
        thread_->join();
        thread_ = nullptr;

        delete parent_pipe_;
    }

    return (sig == zmqpp::signal::ok);
}

void DistWorkerAPI::HandleNetwork() {
    try {
        ForEachValidCommunicator([](std::unique_ptr<Communicator>& comm) {
            comm->Connect();
        });
    }
    catch (...) {
        eptr_ = std::current_exception();
        child_pipe_->send(zmqpp::signal::ko);

        std::cout << "E: exception thrown!" << std::endl;
        goto cleanup;
    }
    child_pipe_->send(zmqpp::signal::ok);

    // Add app_sock to reactor
    info_->reactor_->add( *child_pipe_, std::bind(&DistWorkerAPI::HandleAppMessage, this));

    std::cout << "I: listening to network..." << std::endl;
    while ( !info_->zctx_interrupted_ ) {
        if ( !info_->reactor_->poll ( HEARTBEAT_INTERVAL.count() ) ) {
            ForEachValidCommunicator([](std::unique_ptr<Communicator>& comm) {
                comm->ReactorTimedOut();
            });
        }

        if (!info_->pending_->empty()) {
            // Handle pending messages from network
            HandleNetworkMessages();
        }

        ForEachValidCommunicator([](std::unique_ptr<Communicator>& comm) {
            comm->ReactorServedRequests();
        });
    }

cleanup:
    Cleanup();
    std::cout << "I: Terminated!" << std::endl;
}

void DistWorkerAPI::HandleAppMessage () {
    // The message must have the following format
    // Frame 1: communicator identifier (where to send message?)
    // Frame 2: recipient id
    // Frame 3: application frame(s)

    zmqpp::message msg;
    if ( !child_pipe_->receive(msg)
            || msg.is_signal() ) {
        // Interrupted
        info_->zctx_interrupted_ = true;
        return;
    }

    // Find out where to forward the message
    std::uint8_t comm_id;
    msg.get(comm_id, 0);
    msg.pop_front();

    auto& comm = GetValidCommunicator(comm_id);
    comm.HandleOutgoingMessage(msg);
}

void DistWorkerAPI::HandleNetworkMessages() {
    std::uint8_t comm_id;
    for (auto& msg_p : *(info_->pending_)) {
        // Verify that sender exists
        msg_p->get(comm_id, 0);
        GetValidCommunicator(comm_id);

        child_pipe_->send(*msg_p);
    }
    info_->pending_->clear();
}

void DistWorkerAPI::Cleanup() {
    try {
        // Explicitly delete communicators
        ForEachValidCommunicator([](std::unique_ptr<Communicator>& comm) {
            comm.reset();
        });
        info_->reactor_->remove( *child_pipe_ );

        // Everything cleaned!
        child_pipe_->send(zmqpp::signal::ok);
    }
    catch (...) {
        // Error occured; no gratefull exited
        child_pipe_->send(zmqpp::signal::ko);
    }

    delete child_pipe_;
    delete info_->reactor_;
}

Communicator& DistWorkerAPI::GetValidCommunicator (std::uint8_t comm_id) {
    if (comm_id == 0 || comm_id >= comms_.size()) {
        std::cout << "E: Invalid communicator id: " << comm_id << std::endl;
        assert(false);
    }
    assert( comms_[comm_id] );
    return *comms_[comm_id];
}

void DistWorkerAPI::ForEachValidCommunicator(std::function<void(std::unique_ptr<Communicator>&)> f) {
    for (auto& comm : comms_) {
        if (comm) {
            f(comm);
        }
    }
}

}
