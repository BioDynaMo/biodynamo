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
  static LoggingLevel level_;
};

inline void LWorker() {
  Logger logger("APP-W1", TestWWData::level_);
  DistWorkerAPI api(&TestWWData::ctx_, "W1", TestWWData::level_);

  api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  std::string msg;
  for (size_t i = 0; i < TestWWData::n_messages_; i++) {
    // wait for message
    assert(api.ReceiveDebugMessage(&msg, CommunicatorId::kLeftNeighbour));

    logger.Debug("Received message: ", msg);

    // echo that message
    api.SendDebugMessage(msg, CommunicatorId::kLeftNeighbour);
  }

  // Send stop signal
  assert(api.Stop());
}

inline void RWorker() {
  Logger logger("APP-W2", TestWWData::level_);
  DistWorkerAPI api(&TestWWData::ctx_, "W2", TestWWData::level_);

  api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5500");
  assert(api.Start());

  auto start = std::chrono::high_resolution_clock::now();

  logger.Info("I: Sending ", TestWWData::n_messages_, " messages...");

  std::string msg = "Hello world!";
  for (size_t i = 0; i < TestWWData::n_messages_; i++) {
    // Send first must send the message
    api.SendDebugMessage(msg, CommunicatorId::kRightNeighbour);

    assert(api.ReceiveDebugMessage(&msg, CommunicatorId::kRightNeighbour));

    logger.Debug("Received message: ", msg);
  }

  auto elapsed =
      static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::high_resolution_clock::now() - start)
                              .count()) /
      1000.0;
  logger.Warning("Received ", TestWWData::n_messages_, " replies in ", elapsed,
                 " ms");
  logger.Warning("Time per message: ", elapsed / (TestWWData::n_messages_),
                 " ms");

  // Send stop signal
  assert(api.Stop());
}
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BENCHMARKS_WW_BENCHMARK_H_
