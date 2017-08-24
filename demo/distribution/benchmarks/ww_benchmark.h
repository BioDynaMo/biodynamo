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
  Logger logger("APP-W1");
  DistWorkerAPI api(&TestWWData::ctx_, "W1", TestWWData::verbose_);

  api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  for (size_t i = 0; i < TestWWData::n_messages_; i++) {
    // wait for message
    msg = std::make_unique<zmqpp::message>();
    assert(api.ReceiveMessage(&msg, CommunicatorId::kRightNeighbour));

    if (TestWWData::verbose_) {
      logger.Info("Received message: ", *msg);
    }

    // echo that message
    api.SendMessage(std::move(msg), CommunicatorId::kRightNeighbour);
  }

  // Send stop signal
  assert(api.Stop());
}

inline void LWorker() {
  Logger logger("APP-W2");
  DistWorkerAPI api(&TestWWData::ctx_, "W2", TestWWData::verbose_);

  api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  auto start = std::chrono::high_resolution_clock::now();

  logger.Info("I: Sending ", TestWWData::n_messages_, " messages...");

  zmqpp::message hello_msg;
  hello_msg.push_front("Hello world");

  std::unique_ptr<zmqpp::message> msg;
  for (size_t i = 0; i < TestWWData::n_messages_; i++) {
    // Send first must send the message
    msg = std::make_unique<zmqpp::message>(hello_msg.copy());
    api.SendMessage(std::move(msg), CommunicatorId::kLeftNeighbour);

    msg = std::make_unique<zmqpp::message>();
    assert(api.ReceiveMessage(&msg, CommunicatorId::kLeftNeighbour));

    if (TestWWData::verbose_) {
      logger.Info("Received message: ", *msg);
    }
  }

  auto elapsed =
      static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
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
