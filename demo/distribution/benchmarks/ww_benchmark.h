#ifndef __WW_BENCHMARK__
#define __WW_BENCHMARK__

#include <iostream>
#include <chrono>
#include <string>
#include <thread>

#include <stdlib.h>

#include <zmqpp/zmqpp.hpp>

#include "distribution/dist_worker_api.h"

namespace mdp {

struct Info {
    static zmqpp::context ctx;
    static size_t n_messages;
    static bool verbose;
};

inline void RWorker() {
    DistWorkerAPI api(&Info::ctx, "W1", Info::verbose);

    api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5500");
    assert( api.Start() );

    zmqpp::message msg;
    for (size_t i = 0; i < Info::n_messages; i++) {
        // wait for message
        api.ReceiveMessage(&msg);

        if (Info::verbose) {
            std::cout << "R-APP: received message: " << msg << std::endl;
        }

        // echo that message
        api.SendMessage(msg);
    }

    // Send stop signal
    assert( api.Stop() );
}


inline void LWorker() {
    DistWorkerAPI api (&Info::ctx, "W2", Info::verbose);

    api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5500");
    assert( api.Start() );

    auto start = std::chrono::high_resolution_clock::now();

    std::cout << "I: Sending " << Info::n_messages << " messages..." << std::endl;

    // Send first must send the message
    zmqpp::message initial_msg;
    initial_msg.push_front("Hello world");
    initial_msg.push_front("");
    initial_msg.push_front(
        ToUnderlying(CommunicatorId::kLeftNeighbour)
    );

    zmqpp::message msg = initial_msg.copy();
    for (size_t i = 0; i < Info::n_messages; i++) {
        api.SendMessage(msg);

        // echo that message
        api.ReceiveMessage(&msg);

        // TODO: define some kind of equality
        //assert(initial_msg == msg);

        if (Info::verbose) {
            std::cout << "L-APP: received message: " << msg << std::endl;
        }

    }

    auto elapsed = ((double)std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - start
    ).count()) / 1000.0;
    std::cout << "I: Sent " << Info::n_messages << " messages in " \
        << elapsed  << " ms" << std::endl;
    std::cout << "I: Time per message: " << elapsed / Info::n_messages << " ms" << std::endl;

    // Send stop signal
    assert( api.Stop() );
}

}

#endif //__WW_BENCHMARK__
