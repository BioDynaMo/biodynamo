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

#include "core/parallel_execution/parallel_execution_manager.h"
#include "core/param/command_line_options.h"

namespace bdm {

class ParallelExecutor {
 public:
  ParallelExecutor(int argc, const char** argv) : argc_(argc), argv_(argv) {}

  int Execute(
      const std::function<int(int, const char**, XMLParams*)>& simulate_call) {
    auto options = CommandLineOptions(argc_, argv_);
    std::string xml_file = options.Get<std::string>("xml");
    if (xml_file != "") {
      // MPI_Init needs a non-const version of argv, so we make a deep copy
      char** argv_cpy = (char**)malloc((argc_ + 1) * sizeof(char*));
      for (int i = 0; i < argc_; ++i) {
        size_t length = strlen(argv_[i]) + 1;
        argv_cpy[i] = (char*)malloc(length);
        memcpy(argv_cpy[i], argv_[i], length);
      }
      argv_cpy[argc_] = nullptr;

      int worldsize, provided;
      MPI_Init_thread(&argc_, &argv_cpy, MPI_THREAD_SERIALIZED, &provided);
      if (provided < MPI_THREAD_SERIALIZED) {
        Log::Error("MPI_Init_thread",
                   "The threading support level is lesser than that demanded.");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
      }
      MPI_Comm_size(MPI_COMM_WORLD, &worldsize);

      int myrank;
      MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

      if (myrank == 0) {
        // Delete existing root files
        std::string result_dir = gSystem->GetWorkingDirectory();
        std::stringstream ss;
        ss << "rm " << result_dir << "/*.root";
        if (system(ss.str().c_str()) != 0) {
          Log::Error("main", "Non zero return code with with command '",
                     ss.str(), "'");
        }

        ParallelExecutionManager pem(
            worldsize, xml_file, [&](XMLParams* params) {
              return simulate_call(argc_, argv_, params);
            });
        int ret = pem.Start();
        MPI_Finalize();

        ss.str("");

        // Merge result files of all workers into single ROOT file
        ss << "hadd " << result_dir << "/results.root " << result_dir
           << "/*.root > /dev/null";
        if (system(ss.str().c_str())) {
          Log::Error("main",
                     "An error occured when trying to merge .root files");
        } else {
          std::cout << "Simulation finished successfully. Results are written "
                       "to "
                    << result_dir << "/results.root" << std::endl;
        }
        return ret;
      } else {
        Worker w(myrank, [&](XMLParams* params) {
          return simulate_call(argc_, argv_, params);
        });
        int ret = w.Start();
        MPI_Finalize();
        for (int i = 0; i < argc_; ++i) {
          free(argv_cpy[i]);
        }
        free(argv_cpy);
        return ret;
      }
    } else {
      Log::Error("ParallelExecutor",
                 "No XML file specified in the command line argument: "
                 "--xml=/path/to/xml/file.");
      return 1;
    }
  }

 private:
  int argc_ = 0;
  const char** argv_;
};

}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_PARALLEL_EXECUTOR_H_
