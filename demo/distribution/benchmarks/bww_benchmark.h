#ifndef __BWW_BENCHMARK__
#define __BWW_BENCHMARK__

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <stdlib.h>

#include <zmqpp/zmqpp.hpp>

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
  static bool verbose_;
};

inline void TestBWWClientTask() {
  Client client(&TestBWWData::ctx_, "tcp://127.0.0.1:5555",
                TestBWWData::verbose_);

  std::cout << "I: Sending " << TestBWWData::n_messages_ << " messages..."
            << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  size_t remaining = TestBWWData::n_messages_;

  std::string command, worker;
  zmqpp::message msg;
  while (remaining > 0) {
    msg.push_back("Hello world");

    // Round robin
    worker =
        (remaining % 2 == 0 ? TestBWWData::worker1_ : TestBWWData::worker2_);
    client.Send(worker, msg);

    if (!client.Recv(&command, nullptr, msg)) {
      std::cout << "Interrupted..." << std::endl;
      break;
    }

    if (command == MDPC_NAK) {
      std::cout << "E: invalid worker " << worker << std::endl;
      std::this_thread::sleep_for(HEARTBEAT_INTERVAL);
      continue;
    }

    remaining--;
  }

  auto elapsed = ((double)std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::high_resolution_clock::now() - start)
                      .count()) /
                 1000.0;
  std::cout << "I: Received " << TestBWWData::n_messages_ - remaining
            << " replies in " << elapsed << " ms" << std::endl;
  std::cout << "I: Time per message: "
            << elapsed / (TestBWWData::n_messages_ - remaining) << " ms"
            << std::endl;

  // Send stop signal
  // assert( api.Stop() );
}

inline void TestBWWBrokerTask() {
  Broker broker(&TestBWWData::ctx_, "tcp://*:5555", TestBWWData::verbose_);
  broker.Run();
}

inline void TestBWWWorker1Task() {
  DistWorkerAPI api(&TestBWWData::ctx_, TestBWWData::worker1_,
                    TestBWWData::verbose_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5556");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  std::uint8_t from;
  for (size_t i = 0; i < TestBWWData::n_messages_; i++) {
    // wait for message
    api.ReceiveMessage(msg);

    if (TestBWWData::verbose_) {
      std::cout << "WORKER 1: received message: " << *msg << std::endl;
    }

    msg->get(from, 0);
    msg->pop_front();

    switch (static_cast<CommunicatorId>(from)) {
      case CommunicatorId::kBroker:
        api.SendMessage(msg, CommunicatorId::kRightNeighbour);
        break;
      case CommunicatorId::kRightNeighbour:
        api.SendMessage(msg, CommunicatorId::kBroker);
        break;
      default:
        std::cout << "Error: wrong communicator id" << std::endl;
        assert(api.Stop());
        return;
    }
  }

  // Send stop signal
  assert(api.Stop());
}

inline void TestBWWWorker2Task() {
  DistWorkerAPI api(&TestBWWData::ctx_, TestBWWData::worker2_,
                    TestBWWData::verbose_);

  api.AddBrokerCommunicator("tcp://127.0.0.1:5555");
  api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5556");
  assert(api.Start());

  std::unique_ptr<zmqpp::message> msg;
  std::uint8_t from;
  for (size_t i = 0; i < TestBWWData::n_messages_; i++) {
    // wait for message
    api.ReceiveMessage(msg);

    if (TestBWWData::verbose_) {
      std::cout << "WORKER 2: received message: " << *msg << std::endl;
    }

    msg->get(from, 0);
    msg->pop_front();

    switch (static_cast<CommunicatorId>(from)) {
      case CommunicatorId::kBroker:
        api.SendMessage(msg, CommunicatorId::kLeftNeighbour);
        break;
      case CommunicatorId::kLeftNeighbour:
        api.SendMessage(msg, CommunicatorId::kBroker);
        break;
      default:
        std::cout << "Error: wrong communicator id" << std::endl;
        assert(api.Stop());
        return;
    }
  }

  // Send stop signal
  assert(api.Stop());
}
}

#endif  // __BWW_BENCHMARK__
