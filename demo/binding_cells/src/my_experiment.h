#ifndef MY_EXPERIMENT_H_
#define MY_EXPERIMENT_H_

#include <string>
#include <vector>
#include "experiment.h"

namespace bdm {

class MyExperiment : public Experiment {
 public:
  // The results of interest
  std::vector<int> activity;
  std::vector<int> occupancy;

  MyExperiment(std::string n, std::string b) : Experiment(n, b) {}

  void WriteResultToROOT() { WriteResultToROOTBase(this); }
};

}  // namespace bdm

#endif  // MY_EXPERIMENT_H_
