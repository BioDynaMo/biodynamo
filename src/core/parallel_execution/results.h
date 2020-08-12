#ifndef CORE_PARALLEL_EXECUTION_RESULTS_H_
#define CORE_PARALLEL_EXECUTION_RESULTS_H_

#ifdef USE_MPI

#include <sstream>
#include <string>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"

#include "mpi.h"

namespace bdm {

/// Base class that represents a single experiment
class Results {
 public:
  Results() : name(""), brief("") {}
  Results(std::string n, std::string b) : name(n), brief(b) {}

  virtual ~Results() {}

  /// Write experimental results as an Event within a TTree to ROOT file
  template <typename TResults>
  void WriteResultToROOTBase(TResults *res) {
    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    std::string result_dir = gSystem->GetWorkingDirectory();
    std::stringstream ss;
    ss << result_dir << "/" << name << "_results." << processor_name << "-" << world_rank << ".root";

    // Open a ROOT file specific to this MPI process (might not exist yet)
    TFile tfile(ss.str().c_str(), "UPDATE");

    // If ROOT file existed, get the TTree object
    TTree *tree = static_cast<TTree *>(tfile.Get(name.c_str()));

    // If ROOT file didn't exist before, our TTree* is a nullptr
    if (tree == nullptr) {
      // Create new TTree and pass the name and description to it
      tree = new TTree(name.c_str(), brief.c_str());
      // Write the results to file under the specified branch name
      tree->Branch(name.c_str(), res);
    } else {  // If ROOT file did exist, we should append to existing TTree
      // Make a local copy of the object pointer
      auto *res_ptr = res;
      // Append to branch
      tree->SetBranchAddress(name.c_str(), &res_ptr);
    }

    auto err = tree->Fill();
    if (err == -1) {
      std::cout << "Error occurred when writing results to file!" << std::endl;
    }
    tfile.Write();
  }

 private:
  BDM_CLASS_DEF(Results, 1);
  /// The number of simulation timesteps ran for this experiment
  // int timesteps;

  /// Name of the experiment
  std::string name;  //!

  /// Brief description of the experiment
  std::string brief;  //!
};

}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_PARALLEL_EXECUTION_RESULTS_H_
