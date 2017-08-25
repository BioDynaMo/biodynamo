#ifndef DEMO_DISTRIBUTION_BENCHMARKS_BWW_BENCHMARK_H_
#define DEMO_DISTRIBUTION_BENCHMARKS_BWW_BENCHMARK_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "distribution/broker.h"
#include "distribution/client.h"
#include "distribution/common.h"
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
  Logger logger("Task[Client]");
  Client client(&TestBWWData::ctx_, "tcp://127.0.0.1:5555",
                TestBWWData::level_);

  logger.Info("Sending ", TestBWWData::n_messages_, " messages...");

  auto start = std::chrono::high_resolution_clock::now();
  size_t remaining = TestBWWData::n_messages_;

  zmqpp::message hello_msg;
  hello_msg.push_back("Hello world");

  ClientProtocolCmd command;
  std::unique_ptr<zmqpp::message> msg;
  std::string worker;
  while (remaining > 0) {
    // Round robin
    worker =
        (remaining % 2 == 0 ? TestBWWData::worker1_ : TestBWWData::worker2_);

    msg = std::make_unique<zmqpp::message>(hello_msg.copy());
    client.Send(worker, std::move(msg));

    msg = std::make_unique<zmqpp::message>(hello_msg.copy());
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

  // Send stop signal
  // assert( api.Stop() );
}

inline void TestBWWBrokerTask() {
  Broker broker(&TestBWWData::ctx_, "tcp://*:5555", TestBWWData::level_);
  broker.Run();
}

inline void TestBWWWorker1Task() {
  Logger logger("Task[W1]");
  DistWorkerAPI api(&TestBWWData::ctx_, TestBWWData::worker1_,
                    TestBWWData::level_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5556");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  std::uint8_t from;
  for (size_t i = 0; i < TestBWWData::n_messages_; i++) {
    // wait for message
    api.ReceiveMessage(&msg);

    logger.Debug("Received message: ", *msg);

    msg->get(from, 0);
    msg->pop_front();

    switch (static_cast<CommunicatorId>(from)) {
      case CommunicatorId::kBroker:
        api.SendMessage(std::move(msg), CommunicatorId::kRightNeighbour);
        break;
      case CommunicatorId::kRightNeighbour:
        api.SendMessage(std::move(msg), CommunicatorId::kBroker);
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
  Logger logger("Task[W2]");
  DistWorkerAPI api(&TestBWWData::ctx_, TestBWWData::worker2_,
                    TestBWWData::level_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5556");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  std::uint8_t from;
  for (size_t i = 0; i < TestBWWData::n_messages_; i++) {
    // wait for message
    api.ReceiveMessage(&msg);

    logger.Debug("Received message: ", *msg);

    msg->get(from, 0);
    msg->pop_front();

    switch (static_cast<CommunicatorId>(from)) {
      case CommunicatorId::kBroker:
        api.SendMessage(std::move(msg), CommunicatorId::kLeftNeighbour);
        break;
      case CommunicatorId::kLeftNeighbour:
        api.SendMessage(std::move(msg), CommunicatorId::kBroker);
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
