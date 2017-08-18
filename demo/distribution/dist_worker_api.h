#ifndef __DIST_WORKER_API_H
#define __DIST_WORKER_API_H

#include <algorithm>
#include <array>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "broker_comm.h"
#include "common.h"
#include "worker_comm.h"

namespace bdm {

class DistWorkerAPI {
 public:
  DistWorkerAPI(zmqpp::context* ctx, const std::string identity, bool verbose);
  ~DistWorkerAPI();
  bool Start();

  void AddBrokerCommunicator(const std::string& endpoint);
  void AddLeftNeighbourCommunicator(const std::string& endpoint);
  void AddRightNeighbourCommunicator(const std::string& endpoint);

  void SendMessage(std::unique_ptr<zmqpp::message>& msg, CommunicatorId to);
  bool ReceiveMessage(std::unique_ptr<zmqpp::message>& msg,
                      duration_ms_t timeout = std::chrono::milliseconds(5000));
  bool ReceiveMessage(std::unique_ptr<zmqpp::message>& msg, CommunicatorId from,
                      duration_ms_t timeout = std::chrono::milliseconds(5000));

  bool IsConnected(const CommunicatorId comm);

  bool Stop(bool wait = true, bool force = false);

  std::exception_ptr GetLastException() { return eptr_; }

 private:
  void HandleNetwork();
  void HandleAppMessage();

  void HandleNetworkMessages();
  void ReceiveAllMessages();

  void Cleanup();

  bool IsValidCommunicator(std::uint8_t comm_id);
  Communicator& GetValidCommunicator(std::uint8_t comm__id);
  void ForEachValidCommunicator(
      std::function<void(std::unique_ptr<Communicator>&)> f);

  DistSharedInfo* info_;

  std::array<std::unique_ptr<Communicator>,
             ToUnderlying(CommunicatorId::kCount)>
      comms_;

  std::array<std::deque<std::unique_ptr<zmqpp::message> >,
             ToUnderlying(CommunicatorId::kCount)>
      msg_queues_;
  std::condition_variable msgs_cv_;
  std::mutex msgs_cv_m;

  zmqpp::socket* parent_pipe_;  // Used by computation thread
  zmqpp::socket* child_pipe_;   // Used by background thread
  std::string endpoint_;        // Application endpoint

  std::thread* thread_ = nullptr;      //  Background/Network thread
  std::exception_ptr eptr_ = nullptr;  //  Holds the last exception
};
}  // namespace bdm

#endif  // __DIST_WORKER_API_H
