#ifndef __MDP_BROKER_H__
#define __MDP_BROKER_H__

#include <chrono>
#include <map>
#include <set>
#include <vector>

#include <zmqpp/zmqpp.hpp>

#include "common.hpp"
#include "worker_entry.hpp"

namespace mdp {

// LRU worker comparator
struct LRU {
    // First sort by (heartbeat) expiration time; then by pointer value
    bool operator() (const WorkerEntry* lhs, const WorkerEntry *rhs) const {
        if (lhs->expiry != rhs->expiry) {
            return lhs->expiry < rhs->expiry;
        }
        return lhs < rhs;
    }
};

class Broker {
  public:
    Broker(zmqpp::context *ctx, const std::string& endpoint, const bool verbose);
    ~Broker();
    void Run();

  private:
    void Bind ();
    void HandleMessageWorker (const std::string& identity, zmqpp::message& msg);
    void HandleMessageClient (const std::string& identity, zmqpp::message& msg);
    void Purge ();

    WorkerEntry* GetOrCreateWorker (const std::string& identity);
    void DeleteWorker (WorkerEntry *worker, bool disconnect = true);

    zmqpp::context *ctx_ = nullptr;
    zmqpp::socket *socket_ = nullptr;
    std::string endpoint_;
    std::map <std::string, WorkerEntry*> workers_;
    std::set<WorkerEntry*, LRU> waiting_;

    time_point_t hb_at_;                         //  When to send HEARTBEAT
    bool verbose_;                               //  Print activity to stdout

};

}

#endif // __MDP_BROKER_H__
