#ifndef DEMO_DISTRIBUTION_BROKER_H__
#define DEMO_DISTRIBUTION_BROKER_H__

#include <map>
#include <set>
#include <string>

#include "common.h"
#include "logger.h"
#include "protocol.h"
#include "worker_entry.h"

namespace bdm {

// LRU worker comparator
struct LRU {
  // First sort by (heartbeat) expiration time; then by pointer value
  bool operator()(const WorkerEntry* lhs, const WorkerEntry* rhs) const {
    if (lhs->expiry_ != rhs->expiry_) {
      return lhs->expiry_ < rhs->expiry_;
    }
    return lhs < rhs;
  }
};

class Broker {
 public:
  Broker(zmqpp::context* ctx, const std::string& endpoint, LoggingLevel level);
  ~Broker();
  void Run();

 private:
  void Bind();
  void HandleMessageWorker(const std::string& identity,
                           std::unique_ptr<zmqpp::message> msg);
  void HandleMessageClient(const std::string& identity,
                           std::unique_ptr<zmqpp::message> msg);

  void ReplyToClient(ClientProtocolCmd cmd, const std::string& client_id,
                     std::unique_ptr<zmqpp::message> message = nullptr,
                     const std::string& worker = "");

  void Purge();

  WorkerEntry* GetOrCreateWorker(const std::string& identity);
  void DeleteWorker(WorkerEntry* worker, bool disconnect = true);

  zmqpp::context* ctx_;
  std::unique_ptr<zmqpp::socket> socket_;
  std::string endpoint_;
  std::map<std::string, std::unique_ptr<WorkerEntry> > workers_;
  std::set<WorkerEntry*, LRU> waiting_;

  time_point_t hb_at_;            //  When to send HEARTBEAT
  bool req_termination_ = false;  //  Client asked termination?

  Logger logger_;
};
}  // namespace bdm

#endif  // DEMO_DISTRIBUTION_BROKER_H__
