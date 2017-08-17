#include <chrono>
#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include "broker_comm.h"

namespace bdm {

BrokerCommunicator::BrokerCommunicator (DistSharedInfo *info, const std::string& endpoint)
    : Communicator(info, endpoint, CommunicatorId::kBroker)
    , hb_delay_(duration_ms_t(HEARTBEAT_INTERVAL))
    , hb_rec_delay_(duration_ms_t(HEARTBEAT_INTERVAL)) {

}


BrokerCommunicator::~BrokerCommunicator() {
    SendToBroker(MDPW_DISCONNECT);
}

void BrokerCommunicator::ReactorTimedOut() {
    // Timeout
    if (--hb_liveness_ == 0 ) {
        std::cout << "W: disconnected from broker - retrying..." << std::endl;
        std::this_thread::sleep_for(hb_rec_delay_);
        Connect();
    }
}

void BrokerCommunicator::ReactorServedRequests() {
    //  Send HEARTBEAT if it's time
    if (std::chrono::system_clock::now() > hb_at_) {
        SendToBroker(MDPW_HEARTBEAT);
        hb_at_ = std::chrono::system_clock::now() + hb_delay_;
    }

    // Purge old sockets
    for (auto& socket_p : purge_later_) {
        socket_p->close();
        delete socket_p;
    }
    purge_later_.clear();
}


void BrokerCommunicator::HandleOutgoingMessage(zmqpp::message& msg) {
    if (info_->verbose_) {
        std::cout << "I: sending message to broker: " << msg << std::endl;
    }

    // Check recipient address
    std::string recipient;
    msg.get(recipient, 0);
    assert( !recipient.empty() );

    SendToBroker(MDPW_REPORT, &msg);
}


void BrokerCommunicator::HandleIncomingMessage() {

    auto msg_p = new zmqpp::message();
    if ( !socket_->receive(*msg_p) ) {
        // Interrupted
        info_->zctx_interrupted_ = true;
        return;
    }

    if (info_->verbose_) {
        std::cout << "I: received message from broker: " << *msg_p << std::endl;
    }
    hb_liveness_ = HEARTBEAT_LIVENESS;

    std::string empty, header, command;
    assert (msg_p->parts() >= 3);

    msg_p->get(empty, 0);
    assert(empty == "");
    msg_p->pop_front();

    msg_p->get(header, 0);
    assert(header == MDPW_WORKER);
    msg_p->pop_front();

    msg_p->get(command, 0);
    msg_p->pop_front();

    if (command == MDPW_REQUEST) {
        // Process message from broker
        msg_p->push_front(
            ToUnderlying(comm_id_)
        );
        info_->pending_->push_back(
            std::unique_ptr<zmqpp::message>(msg_p)
        );
    }
    else if (command == MDPW_HEARTBEAT) {
        // Do nothing
    }
    else if (command == MDPW_DISCONNECT) {
        // Reconnect to broker
        Connect();
    }
    else {
        std::cout << "E: invalid input message" << *msg_p << std::endl;
    }
}


void
BrokerCommunicator::Connect() {
    if (socket_) {
        // Lazy remove socket
        info_->reactor_->remove(*socket_);
        purge_later_.push_back(socket_);
    }

    // Create new socket
    socket_ = new zmqpp::socket(*(info_->ctx_), zmqpp::socket_type::dealer);
    socket_->set(zmqpp::socket_option::identity, info_->identity_);
    socket_->connect(endpoint_);

    // Add newly created broker socket to reactor
    info_->reactor_->add(*socket_, std::bind(&BrokerCommunicator::HandleIncomingMessage, this));

    // Register service with broker
    std::cout << "I: connecting to broker at " << endpoint_ << std::endl;
    SendToBroker (MDPW_READY, nullptr, info_->identity_);

    // Heartbeat
    hb_liveness_ = HEARTBEAT_LIVENESS;
    hb_at_ = std::chrono::system_clock::now() + hb_delay_;
}


void
BrokerCommunicator::SendToBroker(const std::string& command, zmqpp::message *message /* = nullptr */,
        const std::string& option /* = "" */) {

    auto msg = message ? message->copy() : zmqpp::message ();

    //  Stack protocol envelope to start of message
    if (!option.empty()) {
        msg.push_front(option);
    }
    msg.push_front(command);
    msg.push_front(MDPW_WORKER);
    msg.push_front("");

    if (info_->verbose_) {
        std::cout << "I: sending " << command << " to broker: " << msg << std::endl;
    }
    socket_->send(msg);
}


void BrokerCommunicator::SetHeartbeatDelay(const duration_ms_t& hb_delay) {
    this->hb_delay_ = hb_delay_;
}


void BrokerCommunicator::SetHeartbeatReconnect(const duration_ms_t& hb_rec_delay) {
    this->hb_rec_delay_ = hb_rec_delay_;
}

}
