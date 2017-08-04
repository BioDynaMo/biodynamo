#include <chrono>
#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include "broker_comm.hpp"

namespace mdp {

BrokerCommunicator::BrokerCommunicator (DistSharedInfo *info, const std::string& endpoint) {

    this->info_ = info;
    this->endpoint_ = endpoint;
    this->hb_delay_ = duration_ms_t(HEARTBEAT_INTERVAL);
    this->hb_rec_delay_ = duration_ms_t(HEARTBEAT_INTERVAL);

    ConnectToBroker();
}


BrokerCommunicator::~BrokerCommunicator() {
    SendToBroker(MDPW_DISCONNECT);

    if (socket_) {
        socket_->close();
        delete socket_;
    }
}

void BrokerCommunicator::RequestTimedOut () {
    // Timeout
    if (--hb_liveness_ == 0 ) {
        std::cout << "W: disconnected from broker - retrying..." << std::endl;
        std::this_thread::sleep_for(hb_rec_delay_);
        ConnectToBroker();
    }
}


void BrokerCommunicator::RequestCompleted() {
    //  Send HEARTBEAT if it's time
    if (std::chrono::system_clock::now() > hb_at_) {
        SendToBroker(MDPW_HEARTBEAT);
        hb_at_ = std::chrono::system_clock::now() + hb_delay_;
    }
    
    // Purge old sockets
    for (auto socket_p : purge_later_) {
        socket_p->close();
        delete socket_p;
    }
    purge_later_.clear();
}


void BrokerCommunicator::HandleAppMessage(zmqpp::message& msg) {
    if (info_->verbose_) {
        std::cout << "I: sending message to broker: " << msg << std::endl;
    }
    SendToBroker(MDPW_REPORT, &msg);
}


void BrokerCommunicator::HandleBrokerMessage() {

    zmqpp::message msg;
    if ( !socket_->receive(msg) ) {
        // Interrupted
        info_->zctx_interrupted_ = true;
        return;
    }

    if (info_->verbose_) {
        std::cout << "I: received message from broker: " << msg << std::endl;
    }
    hb_liveness_ = HEARTBEAT_LIVENESS;

    std::string empty, header, command;
    assert (msg.parts() >= 3);

    msg.get(empty, 0);
    assert(empty == "");
    msg.pop_front();

    msg.get(header, 0);
    assert(header == MDPW_WORKER);
    msg.pop_front();

    msg.get(command, 0);
    msg.pop_front();

    if (command == MDPW_REQUEST) {
        // Delive message to application
        info_->app_socket_->send(msg);
    }
    else if (command == MDPW_HEARTBEAT) {
        // Do nothing
    }
    else if (command == MDPW_DISCONNECT) {
        // Reconnect to broker
        ConnectToBroker();
    }
    else {
        std::cout << "E: invalid input message" << msg << std::endl;
    }
}


void
BrokerCommunicator::ConnectToBroker() {
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
    info_->reactor_->add(*socket_, std::bind(&BrokerCommunicator::HandleBrokerMessage, this));

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


template<typename T>
void BrokerCommunicator::SetSocketOption(const zmqpp::socket_option& option, const T& value) {
    socket_->set(option, value);
}


template<typename T>
T BrokerCommunicator::GetSocketOption(const zmqpp::socket_option& option) {
    T value;
    GetSocketOption(option, &value);
    return value;
}


template<typename T>
void BrokerCommunicator::GetSocketOption(const zmqpp::socket_option& option, T *value) {
    socket_->get(option, *value);
}

}
