#ifndef DEMO_DISTRIBUTION_BENCHMARKS_BWW_BENCHMARK_H_
#define DEMO_DISTRIBUTION_BENCHMARKS_BWW_BENCHMARK_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "distribution/broker.h"
#include "distribution/client.h"
#include "distribution/dist_worker_api.h"

namespace bdm {

struct TestBWWData {
  static zmqpp::context ctx_;
  static std::string worker1_;
  static std::string worker2_;
  static size_t n_messages_;
  static LoggingLevel level_;
};

inline void TestBWWClientTask() {
  Logger logger("Task[Client]", TestBWWData::level_);
  Client client(&TestBWWData::ctx_, "client", "tcp://127.0.0.1:5555",
                TestBWWData::level_);

  logger.Info("Sending ", TestBWWData::n_messages_, " messages...");

  // Spin until both worker are available
  while (!client.CheckWorker(TestBWWData::worker1_) ||
         !client.CheckWorker(TestBWWData::worker2_)) {
    // Don't buzy-wait
    logger.Debug("Waiting for workers...");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Sample message
  zmqpp::message hello_msg;
  hello_msg.push_back("Hello world");

  // Start stopwatch
  auto start = std::chrono::high_resolution_clock::now();
  size_t remaining = TestBWWData::n_messages_;

  ClientProtocolCmd command;
  std::unique_ptr<zmqpp::message> msg;
  std::string worker;
  while (remaining > 0) {
    // Round robin
    worker =
        (remaining % 2 == 0 ? TestBWWData::worker1_ : TestBWWData::worker2_);

    msg = std::make_unique<zmqpp::message>(hello_msg.copy());
    client.SendToWorker(std::move(msg), worker);

    msg = std::make_unique<zmqpp::message>();
    if (!client.Recv(&msg, &command)) {
      logger.Error("Interrupted...");
      break;
    }

    if (command == ClientProtocolCmd::kNak) {
      logger.Warning("Invalid worker ", worker);
      std::this_thread::sleep_for(HEARTBEAT_INTERVAL);
      continue;
    }

    remaining--;
  }

  auto elapsed =
      static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::high_resolution_clock::now() - start)
                              .count()) /
      1000.0;
  logger.Info("Received ", TestBWWData::n_messages_ - remaining, " replies in ",
              elapsed, " ms");
  logger.Info("Time per message: ",
              elapsed / (TestBWWData::n_messages_ - remaining), " ms");

  // Request broker termination
  assert(client.RequestBrokerTermination());
}

inline void TestBWWBrokerTask() {
  Broker broker(&TestBWWData::ctx_, "tcp://*:5555", TestBWWData::level_);
  broker.Run();
}

inline void TestBWWWorker1Task() {
  Logger logger("Task[W1]", TestBWWData::level_);
  DistWorkerAPI api(&TestBWWData::ctx_, TestBWWData::worker1_,
                    TestBWWData::level_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5556");
  assert(api.Start());

  CommunicatorId from;
  std::string msg;
  for (size_t i = 0; i < TestBWWData::n_messages_; i++) {
    // wait for message
    assert(api.ReceiveDebugMessageFromAny(&msg, &from));

    logger.Debug("Received message: ", msg);

    switch (from) {
      case CommunicatorId::kBroker:
        api.SendDebugMessage(msg, CommunicatorId::kRightNeighbour);
        api.ReceiveDebugMessage(&msg, CommunicatorId::kRightNeighbour);
        api.SendDebugMessage(msg, CommunicatorId::kBroker);
        break;
      case CommunicatorId::kRightNeighbour:
        api.SendDebugMessage(msg, CommunicatorId::kRightNeighbour);
        break;
      default:
        logger.Error("Wrong communicator id");
        assert(api.Stop());
        return;
    }
  }

  // Send stop signal
  assert(api.Stop());
}

inline void TestBWWWorker2Task() {
  Logger logger("Task[W2]", TestBWWData::level_);
  DistWorkerAPI api(&TestBWWData::ctx_, TestBWWData::worker2_,
                    TestBWWData::level_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5556");
  assert(api.Start());

  CommunicatorId from;
  std::string msg;
  for (size_t i = 0; i < TestBWWData::n_messages_; i++) {
    // wait for message
    assert(api.ReceiveDebugMessageFromAny(&msg, &from));

    logger.Debug("Received message: ", msg);

    switch (from) {
      case CommunicatorId::kBroker:
        api.SendDebugMessage(msg, CommunicatorId::kLeftNeighbour);
        api.ReceiveDebugMessage(&msg, CommunicatorId::kLeftNeighbour);
        api.SendDebugMessage(msg, CommunicatorId::kBroker);
        break;
      case CommunicatorId::kLeftNeighbour:
        api.SendDebugMessage(msg, CommunicatorId::kLeftNeighbour);
        break;
      default:
        logger.Error("Wrong communicator id");
        assert(api.Stop());
        return;
    }
  }

  // Send stop signal
  assert(api.Stop());
}
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BENCHMARKS_BWW_BENCHMARK_H_
