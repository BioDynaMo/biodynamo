#ifndef COUNT_NEIGHBOR_FUNCTOR_H_
#define COUNT_NEIGHBOR_FUNCTOR_H_

#include "core/agent/agent.h"
#include "core/functor.h"
#include "core/simulation.h"
#include "gtest/gtest.h"

namespace bdm {

// Functor to count how many neighbors are found. To be used with
// ExecutionContext::ForEachNeighbor. It's functionality is wrapped in the
// function GetNeighbors below.
class CountNeighborsFunctor : public Functor<void, Agent*, double> {
 private:
  Agent* query_;
  size_t num_neighbors_;

  void Reset() {
    num_neighbors_ = 0;
    query_ = nullptr;
  }

 public:
  CountNeighborsFunctor() : num_neighbors_(0) {}

  // This is called once for each neighbor that is found
  void operator()(Agent* neighbor, double squared_distance) {
    if (neighbor == query_) {
      return;
    }
#pragma omp atomic
    num_neighbors_ += 1;
  }

  double GetNumNeighbors() { return num_neighbors_; }

  void SetQueryAgent(Agent* query) {
    Reset();
    query_ = query;
  }
};

// Returns the number of agents that are found by the execution context /
// environment in a spherical search region with radius search_radius around the
// search_center. Each agent that is found satisfies
// distance(agent_postion - search_center) < search_radius
inline size_t GetNeighbors(Double3& search_center, double search_radius) {
  // Compute square search Radius
  search_radius *= search_radius;

  // Initialize Functor
  CountNeighborsFunctor cnf;

  // Get execution context
  auto* ctxt = Simulation::GetActive()->GetExecutionContext();

  // Create virtual agent for search
  Cell virtual_agent(2.0);
  virtual_agent.SetPosition(search_center);

  // Get all neighbors
  ctxt->ForEachNeighbor(cnf, virtual_agent, search_radius);
  return cnf.GetNumNeighbors();
}

// This tests the neighbor search and is called in the tests for octree, kdtree,
// and uniform grid. Read the code below to better understand the test. The
// simulation argument can be used to provide a simulation with a defined
// environment. Future environments in 3D space should also call this function
// to verify its functionality.
inline void TestNeighborSearch(Simulation& simulation) {
  // Add three cells at specific positions
  auto* rm = simulation.GetResourceManager();
  auto* scheduler = simulation.GetScheduler();
  auto* cell1 = new Cell(2.0);
  auto* cell2 = new Cell(4.0);
  auto* cell3 = new Cell(2.0);
  cell1->SetPosition({0.0, 0.0, 0.});
  cell2->SetPosition({5, 0, 0});
  cell3->SetPosition({0, -2.5, 0});
  rm->AddAgent(cell1);
  rm->AddAgent(cell2);
  rm->AddAgent(cell3);
  scheduler->Simulate(1);

  // Test if there are three agents in simulation
  EXPECT_EQ(3, rm->GetNumAgents());

  // Define test points to check how many neighbors we find around them.
  // The distances to cells 1, 2, 3 listed as (d1, d2, d3).
  Double3 test_point_1({-0.1, 0.0, 0.0});  // (0.1, 5.1, 2.502)
  Double3 test_point_2({3.5, 0.0, 0.0});   // (3.5, 1.5, 4.30116)
  Double3 test_point_3({0.0, -2.0, 0.0});  // (2, 5.28516, 0.5)
  Double3 test_point_4({0.0, -0.8, 0.0});  // (0.8, 5.0626, 1.7)
  Double3 test_point_5({-2.1, 0.0, 0.0});  // (2.1, 7.1, 3.26497)

  // Test if we find the correct number of agents. The reference solution can
  // be determined by substracting the search_radius from the bracket behind
  // the respective test point and counting the values that are strictly smaller
  // than 0. E.g.:
  // (0.1, 5.1, 2.502) - 2 =  (-1.9, 3.1, 0.502)
  // (-1.9, 3.1, 0.502) < 0 = (1 , 0 , 0) -> result = 1
  double search_radius = 2;
  EXPECT_EQ(1, GetNeighbors(test_point_1, search_radius));
  EXPECT_EQ(1, GetNeighbors(test_point_2, search_radius));
  EXPECT_EQ(1, GetNeighbors(test_point_3, search_radius));
  EXPECT_EQ(2, GetNeighbors(test_point_4, search_radius));
  EXPECT_EQ(0, GetNeighbors(test_point_5, search_radius));
}

}  // namespace bdm

#endif  // COUNT_NEIGHBOR_FUNCTOR_H_