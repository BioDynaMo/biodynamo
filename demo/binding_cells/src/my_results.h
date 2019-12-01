#ifndef MY_RESULTS_H_
#define MY_RESULTS_H_

#include <string>
#include <vector>
#include "core/parallel_execution/results.h"

namespace bdm {

class MyResults : public Results {
 public:
  // The results of interest
  std::vector<int> activity;
  std::vector<int> occupancy;
  int timesteps;
  double foo;

  MyResults() : Results() {}
  MyResults(std::string n, std::string b) : Results(n, b) {}

  void WriteResultToROOT() { WriteResultToROOTBase(this); }
};

}  // namespace bdm

#endif  // MY_RESULTS_H_
