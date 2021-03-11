// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifdef USE_MPI

#include "mpi.h"

#include "TSystem.h"

#include <cstdlib>
#include <fstream>

#include "core/multi_simulation/multi_simulation.h"
#include "core/multi_simulation/multi_simulation_manager.h"
#include "core/multi_simulation/optimization_param.h"
#include "core/param/command_line_options.h"

namespace bdm {

MultiSimulation::MultiSimulation(int argc, const char** argv)
    : argc_(argc), argv_(argv) {
  // MPI_Init needs a non-const version of argv, so we make a deep copy
  argv_copy_ = (char**)malloc((argc_ + 1) * sizeof(char*));
  for (int i = 0; i < argc_; ++i) {
    size_t length = strlen(argv_[i]) + 1;
    argv_copy_[i] = (char*)malloc(length);
    memcpy(argv_copy_[i], argv_[i], length);
  }
  argv_copy_[argc_] = nullptr;
}

MultiSimulation::~MultiSimulation() {
  for (int i = 0; i < argc_; ++i) {
    free(argv_copy_[i]);
  }
  free(argv_copy_);
}

void MultiSimulation::DeleteResultFiles(const std::string& dir) {
  std::stringstream ss;
  ss << "rm -rf " << dir << "/*.root";
  if (system(ss.str().c_str()) != 0) {
    Log::Error("main", "Non zero return code with with command '", ss.str(),
               "'");
  }
}

void MultiSimulation::MergeResultFiles(const std::string& dir) {
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

int MultiSimulation::Execute(const TSimulate& simulate_call) {
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
    MultiSimulationManager pem(worldsize, &default_params, [&](Param* params) {
      simulate_call(argc_, argv_, params);
    });
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

}  // namespace bdm

#endif  // USE_MPI
