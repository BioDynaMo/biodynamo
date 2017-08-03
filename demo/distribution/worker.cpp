#include <chrono>
#include <iostream>

#include <zmqpp/zmqpp.hpp>

#include "worker.hpp"

namespace mdp {

Worker::Worker (zmqpp::context *ctx, const std::string& broker,
            const std::string& identity, bool verbose) {

    this->ctx = ctx;
    this->broker = broker;
    this->identity = identity;
    this->hb_delay = duration_ms_t(2500);       // msecs
    this->hb_rec_delay = duration_ms_t(2500);   // msecs
    this->verbose = verbose;
    
    ConnectToBroker();
}

Worker::~Worker() {
    SendToBroker(MDPW_DISCONNECT);
    if (sock) {
        sock->close();
        delete sock;
    }
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
    sock->send(msg);
}

void
Worker::ConnectToBroker() {
    if (sock) {
        // Delete socket
        poller.remove(*sock);
        sock->close();
        delete sock;
    }
    sock = new zmqpp::socket(*ctx, zmqpp::socket_type::dealer);
    sock->set(zmqpp::socket_option::identity, identity);
    sock->connect(broker);

    std::cout << "I: connecting to broker at " << broker << std::endl;
    
    // Register service with broker
    SendToBroker (MDPW_READY, nullptr, identity);

    // Add socket to poller
    poller.add(*sock, zmqpp::poller::poll_in);
    
    hb_liveness = HEARTBEAT_LIVENESS;
    hb_at = std::chrono::system_clock::now() + hb_delay;
}


bool 
Worker::Recv(zmqpp::message& msg, std::string **reply_to) { 
    //TODO: check if interrupted
    
    std::string empty, header, command;
    while (true) {
        // Wait till heartbeat duration
        poller.poll( hb_delay.count() );  
        
        if (poller.events(*sock) & zmqpp::poller::poll_in) {
            if ( !sock->receive(msg) ) {
                break; // Interrupted
            }
            if (verbose) {
                std::cout << "I: received message from broker: " << msg << std::endl;
            }
            hb_liveness = HEARTBEAT_LIVENESS;

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
                std::string *rep_to = new std::string();
                msg.get(*rep_to, 0);

                if ( !rep_to->empty() ) {
                    *reply_to = rep_to;
                    msg.pop_front();
                }
                else {
                    delete rep_to;
                }

                return true;
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
        else if (--hb_liveness == 0 ) {
            std::cout << "W: disconnected from broker - retrying..." << std::endl; 
            std::this_thread::sleep_for(hb_rec_delay); 
            ConnectToBroker();
        }

        //  Send HEARTBEAT if it's time
        if (std::chrono::system_clock::now() > hb_at) {
            SendToBroker(MDPW_HEARTBEAT);
            hb_at = std::chrono::system_clock::now() + hb_delay;
        }
    
    }

    return false;
}

void Worker::Send (const zmqpp::message& msg, const std::string& reply_to) {
    assert( !reply_to.empty() );

    // Add client address
    zmqpp::message report = msg.copy(); 
    report.push_front(reply_to);

    SendToBroker(MDPW_REPORT, &report);
}

void Worker::SetHeartbeatDelay(std::chrono::milliseconds hb_delay) {
    this->hb_delay = hb_delay;
}


void Worker::SetHeartbeatReconnect(std::chrono::milliseconds hb_rec_delay) {
    this->hb_rec_delay = hb_rec_delay;
}


template<typename T>
void Worker::SetSocketOption(zmqpp::socket_option option, const T& value) {
    sock->set(option, value);
}

template<typename T>
T Worker::GetSocketOption(zmqpp::socket_option option) {
    T value;
    GetSocketOption(option, &value);
    return value;
}

template<typename T>
void Worker::GetSocketOption(zmqpp::socket_option option, T *value) {
    sock->get(option, *value);
}

}

