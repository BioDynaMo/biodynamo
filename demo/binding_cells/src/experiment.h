#ifndef EXPERIMENT_H_
#define EXPERIMENT_H_

#include <string>
#include <vector>
#include "TFile.h"
#include "TTree.h"

namespace bdm {

/// Base class that represents a single experiment
class Experiment {
 public:
  Experiment(std::string n, std::string b) : name(n), brief(b) {}

  /// Write experimental results as an Event within a TTree to ROOT file
  template <typename TExperiment>
  void WriteResultToROOTBase(TExperiment *exp) {
    TFile tfile("simulation.root", "UPDATE");
    TTree *tree = static_cast<TTree *>(tfile.Get(name.c_str()));

    if (!tree) {
      tree = new TTree(name.c_str(), brief.c_str());
      tree->Branch(name.c_str(), exp);
    } else {
      auto *exp_ptr = exp;
      tree->SetBranchAddress(name.c_str(), &exp_ptr);
    }

    tree->Fill();
    tfile.Write();
  }

  /// The number of simulation timesteps ran for this experiment
  int timesteps;

  /// Name of the experiment
  std::string name;  //!

  /// Brief description of the experiment
  std::string brief;  //!
};

}  // namespace bdm

#endif  // EXPERIMENT_H_
