#ifndef DEMO_DISTRIBUTION_BENCHMARKS_WW_HALO_BENCHMARK_H_
#define DEMO_DISTRIBUTION_BENCHMARKS_WW_HALO_BENCHMARK_H_

#include "Rtypes.h"
#include <zmqpp/zmqpp.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "distribution/dist_worker_api.h"

namespace bdm {

struct TestWWHaloData {
  static zmqpp::context ctx_;
  static size_t n_messages_;
  static LoggingLevel level_;
};

class Payload {
 public:
  Payload() { }
  Payload(const std::string& value) : value_(value) { }
  virtual ~Payload() { }

  std::string value_;

  ClassDef(Payload, 1);
};

inline void Worker(std::string identity, CommunicatorId other) {
  Logger logger("APP-" + identity, TestWWHaloData::level_);
  DistWorkerAPI api(&TestWWHaloData::ctx_, identity, TestWWHaloData::level_);

  if (other == CommunicatorId::kLeftNeighbour) {
    api.AddLeftNeighbourCommunicator("tcp://127.0.0.1:5500");
  }
  else {
    api.AddRightNeighbourCommunicator("tcp://127.0.0.1:5500");
  }
  assert(api.Start());

  Payload req (identity + "-Request"), rep (identity + "-Report");
  std::unique_ptr<Payload> recv_payload;

  AppProtocolCmd cmd;
  size_t to_send = TestWWHaloData::n_messages_;
  size_t to_reply = TestWWHaloData::n_messages_;

  auto start = std::chrono::high_resolution_clock::now();

  // Send halo region request
  logger.Info("Sending ", to_send, " requests..");
  api.SendHaloRegion(req, other, AppProtocolCmd::kRequestHaloRegion);

  while(to_send > 0 || to_reply > 0) {
    // Wait for halo request/report
    assert( api.ReceiveHaloRegion(&recv_payload, other, &cmd) );

    logger.Debug("Received message: ", recv_payload->value_);

    switch(cmd) {
      case AppProtocolCmd::kRequestHaloRegion:
        to_reply--;
        // Reply to request
        logger.Debug("Reply to request...");
        api.SendHaloRegion(rep, other, AppProtocolCmd::kReportHaloRegion);
        break;
      case AppProtocolCmd::kReportHaloRegion:
        // Send next halo request
        if (--to_send > 0) {
          logger.Debug("Sending request..");
          api.SendHaloRegion(req, other, AppProtocolCmd::kRequestHaloRegion);
        }
        break;
      default:
        logger.Error("Invalid command: ", cmd);
    }

  }

  if (other == CommunicatorId::kLeftNeighbour) {
    auto elapsed =
        static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
                                std::chrono::high_resolution_clock::now() - start)
                                .count()) /
        1000.0;
    logger.Warning("Received ", TestWWHaloData::n_messages_, " replies in ", elapsed,
                   " ms");
    logger.Warning("Time per message: ", elapsed / (TestWWHaloData::n_messages_),
                   " ms");
  }

  // Send stop signal
  assert(api.Stop());
}
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BENCHMARKS_WW_BENCHMARK_H_
