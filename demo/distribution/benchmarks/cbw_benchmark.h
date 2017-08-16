#ifndef __CBW_BENCHMARK__
#define __CBW_BENCHMARK__

#include <iostream>
#include <chrono>
#include <string>
#include <thread>

#include <stdlib.h>

#include <zmqpp/zmqpp.hpp>

#include "distribution/broker.h"
#include "distribution/client.h"
#include "distribution/dist_worker_api.h"

namespace mdp {

struct Info {
    static zmqpp::context ctx;
    static std::string worker;
    static size_t n_messages;
    static bool verbose;
};

inline void ClientTask() {
    Client client (&Info::ctx, "tcp://127.0.0.1:5555", Info::verbose);

    std::cout << "I: Sending " << Info::n_messages << " messages..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    size_t remaining = Info::n_messages;

    std::string command;
    zmqpp::message msg;
    while (remaining > 0) {
        msg.push_back("Hello world");

        client.Send(Info::worker, msg);

        if ( !client.Recv(&command, nullptr, msg) ) {
            std::cout << "Interrupted..." << std::endl;
            break;
        }

        if (command == MDPC_NAK) {
            std::cout << "E: invalid worker " << Info::worker << std::endl;
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

inline void WorkerTask() {
    DistWorkerAPI api (&Info::ctx, Info::worker, Info::verbose);

    api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
    assert( api.Start() );

    zmqpp::message msg;
    for (size_t i = 0; i < Info::n_messages; i++) {
        // wait for message
        api.ReceiveMessage(&msg);

        if (Info::verbose) {
            std::cout << "WORKER: received message: " << msg << std::endl;
        }

        // echo that message
        api.SendMessage(msg);
    }

    // Send stop signal
    assert( api.Stop() );
}

}

#endif // __CBW_BENCHMARK__
