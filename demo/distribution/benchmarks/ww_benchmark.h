#ifndef __WW_BENCHMARK__
#define __WW_BENCHMARK__

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <stdlib.h>

#include <zmqpp/zmqpp.hpp>

#include "distribution/dist_worker_api.h"

namespace bdm {

struct Info {
  static zmqpp::context ctx;
  static size_t n_messages;
  static bool verbose;
};

inline void RWorker() {
  DistWorkerAPI api(&Info::ctx, "W1", Info::verbose);

  api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  for (size_t i = 0; i < Info::n_messages; i++) {
    // wait for message
    assert(api.ReceiveMessage(msg, CommunicatorId::kRightNeighbour));

    if (Info::verbose) {
      std::cout << "R-APP: received message: " << *msg << std::endl;
    }

    // echo that message
    api.SendMessage(msg, CommunicatorId::kRightNeighbour);
  }

  // Send stop signal
  assert(api.Stop());
}

inline void LWorker() {
  DistWorkerAPI api(&Info::ctx, "W2", Info::verbose);

  api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  auto start = std::chrono::high_resolution_clock::now();

  std::cout << "I: Sending " << Info::n_messages << " messages..." << std::endl;

  // Send first must send the message
  auto msg = std::make_unique<zmqpp::message>();
  msg->push_front("Hello world");
  msg->push_front("");

  for (size_t i = 0; i < Info::n_messages; i++) {
    api.SendMessage(msg, CommunicatorId::kLeftNeighbour);

    // echo that message
    assert(api.ReceiveMessage(msg, CommunicatorId::kLeftNeighbour));

    // TODO: define some kind of equality
    // assert(initial_msg == msg);

    if (Info::verbose) {
      std::cout << "L-APP: received message: " << *msg << std::endl;
    }
  }

  auto elapsed = ((double)std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::high_resolution_clock::now() - start)
                      .count()) /
                 1000.0;
  std::cout << "I: Sent " << Info::n_messages << " messages in " << elapsed
            << " ms" << std::endl;
  std::cout << "I: Time per message: " << elapsed / Info::n_messages << " ms"
            << std::endl;

  // Send stop signal
  assert(api.Stop());
}
}

#endif  //__WW_BENCHMARK__
