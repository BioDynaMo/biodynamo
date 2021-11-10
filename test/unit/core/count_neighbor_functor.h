#ifndef COUNT_NEIGHBOR_FUNCTOR_H_
#define COUNT_NEIGHBOR_FUNCTOR_H_

#include "core/agent/agent.h"
#include "core/functor.h"
#include "core/simulation.h"

namespace bdm {

// Functor to count how many neighbors are found. To be used with
// ExecutionContext::ForEachNeighbor. It's functionality is wrapped in the
// inline function GetNeighbors below.
class CountNeighborsFunctor : public Functor<void, Agent *, double> {
 private:
  Agent *query_;
  size_t num_neighbors_;

  void Reset() {
    num_neighbors_ = 0;
    query_ = nullptr;
  }

 public:
  CountNeighborsFunctor() : num_neighbors_(0) {}

  // This is called once for each neighbor that is found
  void operator()(Agent *neighbor, double squared_distance) {
    if (neighbor == query_) {
      return;
    }
#pragma omp atomic
    num_neighbors_ += 1;
  }

  double GetNumNeighbors() { return num_neighbors_; }

  void SetQueryAgent(Agent *query) {
    Reset();
    query_ = query;
  }
};

// Returns the number of agents that are found by the execution context /
// environment in a spherical search region with radius search_radius around the
// search_center. Each agent that is found satisfies
// distance(agent_postion - search_center) < search_radius
inline size_t GetNeighbors(Double3 &search_center, double search_radius) {
  // Compute square search Radius
  search_radius *= search_radius;

  // Initialize Functor
  CountNeighborsFunctor cnf;

  // Get execution context
  auto *ctxt = Simulation::GetActive()->GetExecutionContext();

  // Create virtual agent for search
  Cell virtual_agent(2.0);
  virtual_agent.SetPosition(search_center);

  // Get all neighbors
  ctxt->ForEachNeighbor(cnf, virtual_agent, search_radius);
  return cnf.GetNumNeighbors();
}

}  // namespace bdm

#endif  // COUNT_NEIGHBOR_FUNCTOR_H_