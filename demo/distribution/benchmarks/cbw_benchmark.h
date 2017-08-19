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
  static bool verbose_;
};

inline void ClientTask() {
  Client client(&TestCBWData::ctx_, "tcp://127.0.0.1:5555",
                TestCBWData::verbose_);

  std::cout << "I: Sending " << TestCBWData::n_messages_ << " messages..."
            << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  size_t remaining = TestCBWData::n_messages_;

  std::string command;
  zmqpp::message msg;
  while (remaining > 0) {
    msg.push_back("Hello world");

    client.Send(TestCBWData::worker_, msg);

    if (!client.Recv(&command, nullptr, msg)) {
      std::cout << "Interrupted..." << std::endl;
      break;
    }

    if (command == MDPC_NAK) {
      std::cout << "E: invalid worker_ " << TestCBWData::worker_ << std::endl;
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
  std::cout << "I: Received " << TestCBWData::n_messages_ - remaining
            << " replies in " << elapsed << " ms" << std::endl;
  std::cout << "I: Time per message: "
            << elapsed / (TestCBWData::n_messages_ - remaining) << " ms"
            << std::endl;

  // Send stop signal
  // assert( api.Stop() );
}

inline void BrokerTask() {
  Broker broker(&TestCBWData::ctx_, "tcp://*:5555", TestCBWData::verbose_);
  broker.Run();
}

inline void WorkerTask() {
  DistWorkerAPI api(&TestCBWData::ctx_, TestCBWData::worker_,
                    TestCBWData::verbose_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  for (size_t i = 0; i < TestCBWData::n_messages_; i++) {
    // wait for message
    api.ReceiveMessage(msg);

    if (TestCBWData::verbose_) {
      std::cout << "worker_: received message: " << *msg << std::endl;
    }

    // echo that message
    api.SendMessage(msg, CommunicatorId::kBroker);
  }

  // Send stop signal
  assert(api.Stop());
}
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BENCHMARKS_CBW_BENCHMARK_H_
