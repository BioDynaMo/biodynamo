#ifndef DEMO_DISTRIBUTION_BENCHMARKS_CBW_BENCHMARK_H_
#define DEMO_DISTRIBUTION_BENCHMARKS_CBW_BENCHMARK_H_

#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "distribution/broker.h"
#include "distribution/client.h"
#include "distribution/dist_worker_api.h"

namespace bdm {

struct TestCBWData {
  static zmqpp::context ctx_;
  static std::string worker_;
  static size_t n_messages_;
  static LoggingLevel level_;
};

inline void ClientTask() {
  Logger logger("Task[Client]", TestCBWData::level_);
  Client client(&TestCBWData::ctx_, "tcp://127.0.0.1:5555",
                TestCBWData::level_);

  logger.Info("Sending ", TestCBWData::n_messages_, " messages...");

  auto start = std::chrono::high_resolution_clock::now();
  size_t remaining = TestCBWData::n_messages_;

  zmqpp::message hello_msg;
  hello_msg.push_back("Hello world");

  ClientProtocolCmd command;
  std::unique_ptr<zmqpp::message> msg;
  while (remaining > 0) {
    msg = std::make_unique<zmqpp::message>(hello_msg.copy());
    client.Send(TestCBWData::worker_, std::move(msg));

    msg = std::make_unique<zmqpp::message>();
    if (!client.Recv(&msg, &command)) {
      logger.Error("Interrupted...");
      break;
    }

    logger.Debug("Received: ", *msg);

    if (command == ClientProtocolCmd::kNak) {
      logger.Warning("Invalid worker ", TestCBWData::worker_);
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

  logger.Info("Received ", TestCBWData::n_messages_ - remaining, " replies in ",
              elapsed, " ms");
  logger.Info("Time per message: ",
              elapsed / (TestCBWData::n_messages_ - remaining), " ms");

  // Send stop signal
  // assert( api.Stop() );
}

inline void BrokerTask() {
  Broker broker(&TestCBWData::ctx_, "tcp://*:5555", TestCBWData::level_);
  broker.Run();
}

inline void WorkerTask() {
  Logger logger("Task[W1]", TestCBWData::level_);
  DistWorkerAPI api(&TestCBWData::ctx_, TestCBWData::worker_,
                    TestCBWData::level_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  for (size_t i = 0; i < TestCBWData::n_messages_; i++) {
    // wait for message
    api.ReceiveMessage(&msg);

    logger.Debug("Received message: ", *msg);

    // echo that message
    msg->push_front("client");
    api.SendMessage(std::move(msg), CommunicatorId::kBroker);
  }

  // Send stop signal
  assert(api.Stop());
}
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BENCHMARKS_CBW_BENCHMARK_H_
