#include <chrono>
#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include "worker.hpp"

namespace mdp {

Worker::Worker (zmqpp::context *ctx, const std::string& broker_addr,
            const std::string& identity, bool verbose) {

    this->ctx = ctx;
    this->broker_addr = broker_addr;
    this->identity = identity;
    this->hb_delay = duration_ms_t(2500);
    this->hb_rec_delay = duration_ms_t(2500);
    this->verbose = verbose;
}

Worker::~Worker() {
    SendToBroker(MDPW_DISCONNECT);
    if (broker_sock) {
        broker_sock->close();
        delete broker_sock;
    }
}

void Worker::StartThread(const std::string& app_addr) {
    this->app_addr = app_addr;

    worker_thread = new std::thread(&Worker::HandleNetwork, this);
    std::cout << "I: started thread with id " << worker_thread->get_id() << std::endl;
}

void Worker::HandleNetwork() {
    // Create local/application socket
    app_sock = new zmqpp::socket(*ctx, zmqpp::socket_type::pair);
    app_sock->connect(app_addr);

    std::cout << "I: connecting to app at " << app_addr << std::endl;

    ConnectToBroker();

    // Add app_sock to reactor
    // NOTE: broker_sock has been added already
    reactor.add(*app_sock, std::bind(&Worker::HandleAppMessage, this),
            zmqpp::poller::poll_in);

    while ( !zctx_interrupted ) {
        if ( !reactor.poll (hb_delay.count()) ) {
            // Timeout
            if (--hb_liveness == 0 ) {
                std::cout << "W: disconnected from broker - retrying..." << std::endl;
                std::this_thread::sleep_for(hb_rec_delay);
                ConnectToBroker();
            }
        }

        //  Send HEARTBEAT if it's time
        if (std::chrono::system_clock::now() > hb_at) {
            SendToBroker(MDPW_HEARTBEAT);
            hb_at = std::chrono::system_clock::now() + hb_delay;
        }

        // Purge old sockets
        for (auto socket_p : purge_later) {
            socket_p->close();
            delete socket_p;
        }
        purge_later.clear();

    }
}

void Worker::HandleAppMessage() {

    zmqpp::message msg;
    if ( !app_sock->receive(msg) ) {
        // Interrupted
        zctx_interrupted = true;
        return;
    }

    // The message received must have the following format
    // Frame 1: sender id
    // Frame 2: application frame(s)

    // Check recepient address
    std::string recepient;
    msg.get(recepient, 0);
    assert( !recepient.empty() );

    if (verbose) {
        std::cout << "I: sending message to broker: " << msg << std::endl;
    }
    SendToBroker(MDPW_REPORT, &msg);
}

void Worker::HandleBrokerMessage() {

    zmqpp::message msg;
    if ( !broker_sock->receive(msg) ) {
        // Interrupted
        zctx_interrupted = true;
        return;
    }

    if (verbose) {
        std::cout << "I: received message from broker: " << msg << std::endl;
    }
    hb_liveness = HEARTBEAT_LIVENESS;

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
        app_sock->send(msg);
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
Worker::ConnectToBroker() {
    if (broker_sock) {
        // Delete socket
        reactor.remove(*broker_sock);
        // Purge socket later
        purge_later.push_back(broker_sock);
    }

    // Create new socket
    broker_sock = new zmqpp::socket(*ctx, zmqpp::socket_type::dealer);
    broker_sock->set(zmqpp::socket_option::identity, identity);
    broker_sock->connect(broker_addr);

    // Add newly created broker socket to reactor
    reactor.add(*broker_sock, std::bind(&Worker::HandleBrokerMessage, this));

    // Register service with broker
    std::cout << "I: connecting to broker at " << broker_addr << std::endl;
    SendToBroker (MDPW_READY, nullptr, identity);

    // Heartbeat
    hb_liveness = HEARTBEAT_LIVENESS;
    hb_at = std::chrono::system_clock::now() + hb_delay;
}


void
Worker::SendToBroker(const std::string& command, zmqpp::message *message /* = nullptr */,
        const std::string& option /* = "" */) {

    auto msg = message ? message->copy() : zmqpp::message ();

    //  Stack protocol envelope to start of message
    if (!option.empty()) {
        msg.push_front(option);
    }
    msg.push_front(command);
    msg.push_front(MDPW_WORKER);
    msg.push_front("");

    if (verbose) {
        std::cout << "I: sending " << command << " to broker: " << msg << std::endl;
    }
    broker_sock->send(msg);
}


void Worker::SetHeartbeatDelay(const duration_ms_t& hb_delay) {
    this->hb_delay = hb_delay;
}


void Worker::SetHeartbeatReconnect(const duration_ms_t& hb_rec_delay) {
    this->hb_rec_delay = hb_rec_delay;
}


template<typename T>
void Worker::SetSocketOption(const zmqpp::socket_option& option, const T& value) {
    broker_sock->set(option, value);
}


template<typename T>
T Worker::GetSocketOption(const zmqpp::socket_option& option) {
    T value;
    GetSocketOption(option, &value);
    return value;
}


template<typename T>
void Worker::GetSocketOption(const zmqpp::socket_option& option, T *value) {
    broker_sock->get(option, *value);
}

}
