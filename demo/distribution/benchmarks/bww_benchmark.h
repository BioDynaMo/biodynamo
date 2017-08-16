#ifndef __BWW_BENCHMARK__
#define __BWW_BENCHMARK__

#include <iostream>
#include <chrono>
#include <string>
#include <thread>

#include <stdlib.h>

#include <zmqpp/zmqpp.hpp>

#include "distribution/common.h"
#include "distribution/broker.h"
#include "distribution/client.h"
#include "distribution/dist_worker_api.h"

namespace mdp {

struct Info {
    static zmqpp::context ctx;
    static std::string worker1;
    static std::string worker2;
    static size_t n_messages;
    static bool verbose;
};

inline void ClientTask() {
    Client client (&Info::ctx, "tcp://127.0.0.1:5555", Info::verbose);

    std::cout << "I: Sending " << Info::n_messages << " messages..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    size_t remaining = Info::n_messages;

    std::string command, worker;
    zmqpp::message msg;
    while (remaining > 0) {
        msg.push_back("Hello world");

        // Round robin
        worker = (remaining % 2 == 0 ? Info::worker1 : Info::worker2);
        client.Send(worker, msg);

        if ( !client.Recv(&command, nullptr, msg) ) {
            std::cout << "Interrupted..." << std::endl;
            break;
        }

        if (command == MDPC_NAK) {
            std::cout << "E: invalid worker " << worker << std::endl;
            std::this_thread::sleep_for( HEARTBEAT_INTERVAL );
            continue;
        }

        remaining--;
    }

    auto elapsed = ((double)std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start
    ).count()) / 1000.0;
    std::cout << "I: Received " << Info::n_messages - remaining << " replies in " \
        << elapsed  << " ms" << std::endl;
    std::cout << "I: Time per message: " << elapsed / (Info::n_messages - remaining) \
        << " ms" << std::endl;

    // Send stop signal
    //assert( api.Stop() );
}

inline void BrokerTask() {
    Broker broker (&Info::ctx, "tcp://*:5555", Info::verbose);
    broker.Run();
}

inline void Worker1Task() {
    DistWorkerAPI api (&Info::ctx, Info::worker1, Info::verbose);

    api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
    api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5556");
    assert( api.Start() );

    zmqpp::message msg;
    std::uint8_t from; 
    for (size_t i = 0; i < Info::n_messages; i++) {
        // wait for message
        api.ReceiveMessage(&msg);

        if (Info::verbose) {
            std::cout << "WORKER 1: received message: " << msg << std::endl;
        }

        msg.get(from, 0);
        msg.pop_front();

        switch(static_cast<CommunicatorId>(from)) {
            case CommunicatorId::kBroker:
                msg.push_front(ToUnderlying(CommunicatorId::kRightNeighbour));
                break;
            case CommunicatorId::kRightNeighbour:
                msg.push_front(ToUnderlying(CommunicatorId::kBroker));
                break;
            default:
                std::cout << "Error: wrong communicator id" << std::endl;
                assert( api.Stop() );
                return;
        }

        api.SendMessage(msg);
    }

    // Send stop signal
    assert( api.Stop() );
}

inline void Worker2Task() {
    DistWorkerAPI api (&Info::ctx, Info::worker2, Info::verbose);

    api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
    api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5556");
    assert( api.Start() );

    zmqpp::message msg;
    std::uint8_t from; 
    for (size_t i = 0; i < Info::n_messages; i++) {
        // wait for message
        api.ReceiveMessage(&msg);

        if (Info::verbose) {
            std::cout << "WORKER 2: received message: " << msg << std::endl;
        }

        msg.get(from, 0);
        msg.pop_front();

        switch(static_cast<CommunicatorId>(from)) {
            case CommunicatorId::kBroker:
                msg.push_front(ToUnderlying(CommunicatorId::kLeftNeighbour));
                break;
            case CommunicatorId::kLeftNeighbour:
                msg.push_front(ToUnderlying(CommunicatorId::kBroker));
                break;
            default:
                std::cout << "Error: wrong communicator id" << std::endl;
                assert( api.Stop() );
                return;

        }

        api.SendMessage(msg);
    }

    // Send stop signal
    assert( api.Stop() );
}


}

#endif // __BWW_BENCHMARK__
