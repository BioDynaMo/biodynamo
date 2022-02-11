// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "TROOT.h"
#include "TSystem.h"

#include <cstdlib>
#include <fstream>

#include "core/analysis/time_series.h"
#include "core/multi_simulation/database.h"
#include "core/multi_simulation/multi_simulation.h"
#include "core/multi_simulation/multi_simulation_manager.h"
#include "core/multi_simulation/optimization_param.h"
#include "core/param/command_line_options.h"

namespace bdm {
namespace experimental {

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

MultiSimulation::MultiSimulation(int argc, const char** argv,
                                 const TimeSeries& real)
    : MultiSimulation(argc, argv) {
  // Register the real data to the database
  auto* db = Database::GetInstance();
  db->data_ = real;
}

MultiSimulation::~MultiSimulation() {
  for (int i = 0; i < argc_; ++i) {
    free(argv_copy_[i]);
  }
  free(argv_copy_);
}

int MultiSimulation::Execute(const TSimulate& simulate_call) {
  int worldsize, provided, myrank;
  MPI_Init_thread(&argc_, &argv_copy_, MPI_THREAD_MULTIPLE, &provided);
  if (provided < MPI_THREAD_MULTIPLE) {
    Log::Error("MPI_Init_thread",
               "The threading support level is lesser than that demanded.");
    MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
  }
  MPI_Comm_size(MPI_COMM_WORLD, &worldsize);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  ROOT::EnableThreadSafety();

  int status;
  if (myrank == 0) {
    // Make a copy of the default parameters
    Simulation simulation(argc_, argv_);
    Param default_params = *(simulation.GetParam());

    // Start the Master routine
    MultiSimulationManager pem(
        worldsize, &default_params, [&](Param* params, TimeSeries* result) {
          return simulate_call(argc_, argv_, result, params);
        });

    status = pem.Start();
  } else {
#ifdef BDM_USE_OMP
    omp_set_num_threads(2);
#endif  // BDM_USE_OMP
    // Start the Worker routine (`params` to be received by Master)
    Worker w(myrank, [&](Param* params, TimeSeries* result) {
      return simulate_call(argc_, argv_, result, params);
    });
    status = w.Start();
  }

  MPI_Finalize();
  return status;
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MPI
