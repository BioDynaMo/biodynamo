// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_PARALLEL_EXECUTOR_H_
#define CORE_PARALLEL_EXECUTOR_H_

#ifdef USE_MPI

#include "mpi.h"

#include "TSystem.h"

#include <cstdlib>
#include <fstream>

#include "core/parallel_execution/optimization_param.h"
#include "core/parallel_execution/parallel_execution_manager.h"
#include "core/param/command_line_options.h"

namespace bdm {

// Signature of the Simulate() call
using TSimulate = std::function<int(int, const char**, Param*)>;

class ParallelExecutor {
 public:
  ParallelExecutor(int argc, const char** argv) : argc_(argc), argv_(argv) {
    // MPI_Init needs a non-const version of argv, so we make a deep copy
    argv_copy_ = (char**)malloc((argc_ + 1) * sizeof(char*));
    for (int i = 0; i < argc_; ++i) {
      size_t length = strlen(argv_[i]) + 1;
      argv_copy_[i] = (char*)malloc(length);
      memcpy(argv_copy_[i], argv_[i], length);
    }
    argv_copy_[argc_] = nullptr;
  }

  ~ParallelExecutor() {
    for (int i = 0; i < argc_; ++i) {
      free(argv_copy_[i]);
    }
    free(argv_copy_);
  }

  void DeleteResultFiles(const std::string& dir) {
    std::stringstream ss;
    ss << "rm -rf " << dir << "/*.root";
    if (system(ss.str().c_str()) != 0) {
      Log::Error("main", "Non zero return code with with command '", ss.str(),
                 "'");
    }
  }

  void MergeResultFiles(const std::string& dir) {
    std::stringstream ss;
    ss << "hadd " << dir << "/results.root " << dir << "/*.root > /dev/null";
    if (system(ss.str().c_str())) {
      Log::Error("main", "An error occured when trying to merge .root files");
    } else {
      std::cout << "Simulation finished successfully. Results are written "
                   "to "
                << dir << "/results.root" << std::endl;
    }
  }

  int Execute(const TSimulate& simulate_call) {
    int worldsize, provided, myrank;
    MPI_Init_thread(&argc_, &argv_copy_, MPI_THREAD_SERIALIZED, &provided);
    if (provided < MPI_THREAD_SERIALIZED) {
      Log::Error("MPI_Init_thread",
                 "The threading support level is lesser than that demanded.");
      MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &worldsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    int status;
    if (myrank == 0) {
      // Delete existing root files
      std::string result_dir = gSystem->GetWorkingDirectory();
      DeleteResultFiles(result_dir);

      // Make a copy of the default parameters
      Simulation simulation(argc_, argv_);
      Param default_params = *(simulation.GetParam());

      // Start the Master routine
      ParallelExecutionManager pem(
          worldsize, &default_params,
          [&](Param* params) { simulate_call(argc_, argv_, params); });
      status = pem.Start();

      // Merge result files of all workers into single ROOT file
      MergeResultFiles(result_dir);
    } else {
      // Start the Worker routine (`params` to be received by Master)
      Worker w(myrank, [&](Param* params) {
        return simulate_call(argc_, argv_, params);
      });
      status = w.Start();
    }

    MPI_Finalize();
    return status;
  }

 private:
  int argc_ = 0;
  const char** argv_ = nullptr;
  char** argv_copy_ = nullptr;
};

}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_PARALLEL_EXECUTOR_H_
