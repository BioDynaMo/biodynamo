#ifndef DEMO_DISTRIBUTION_BENCHMARKS_WW_BENCHMARK_H_
#define DEMO_DISTRIBUTION_BENCHMARKS_WW_BENCHMARK_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "distribution/dist_worker_api.h"

namespace bdm {

struct TestWWData {
  static zmqpp::context ctx_;
  static size_t n_messages_;
  static bool verbose_;
};

inline void RWorker() {
  DistWorkerAPI api(&TestWWData::ctx_, "W1", TestWWData::verbose_);

  api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  for (size_t i = 0; i < TestWWData::n_messages_; i++) {
    // wait for message
    assert(api.ReceiveMessage(msg, CommunicatorId::kRightNeighbour));

    if (TestWWData::verbose_) {
      std::cout << "R-APP: received message: " << *msg << std::endl;
    }

    // echo that message
    api.SendMessage(msg, CommunicatorId::kRightNeighbour);
  }

  // Send stop signal
  assert(api.Stop());
}

inline void LWorker() {
  DistWorkerAPI api(&TestWWData::ctx_, "W2", TestWWData::verbose_);

  api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  auto start = std::chrono::high_resolution_clock::now();

  std::cout << "I: Sending " << TestWWData::n_messages_ << " messages..."
            << std::endl;

  // Send first must send the message
  auto msg = std::make_unique<zmqpp::message>();
  msg->push_front("Hello world");
  msg->push_front("");

  for (size_t i = 0; i < TestWWData::n_messages_; i++) {
    api.SendMessage(msg, CommunicatorId::kLeftNeighbour);

    // echo that message
    assert(api.ReceiveMessage(msg, CommunicatorId::kLeftNeighbour));

    // TODO(kkanellis): define some kind of equality
    // assert(initial_msg == msg);

    if (TestWWData::verbose_) {
      std::cout << "L-APP: received message: " << *msg << std::endl;
    }
  }

  auto elapsed = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::high_resolution_clock::now() - start)
                      .count()) /
                 1000.0;
  std::cout << "I: Sent " << TestWWData::n_messages_ << " messages in "
            << elapsed << " ms" << std::endl;
  std::cout << "I: Time per message: " << elapsed / TestWWData::n_messages_
            << " ms" << std::endl;

  // Send stop signal
  assert(api.Stop());
}
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BENCHMARKS_WW_BENCHMARK_H_
