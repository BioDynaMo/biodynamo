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
#include "binding_cells.h"
#include "core/util/timing.h"

#include "TSystem.h"

#include "mpi.h"

#include <cstdlib>
#include <fstream>

int main(int argc, const char** argv) {
  auto options = bdm::CommandLineOptions(argc, argv);
  std::string xml_file = options.Get<std::string>("xml");
  if (xml_file != "") {
    // MPI_Init needs a non-const version of argv, so we make a deep copy
    char** argv_cpy = (char**)malloc((argc + 1) * sizeof(char*));
    for (int i = 0; i < argc; ++i) {
      size_t length = strlen(argv[i]) + 1;
      argv_cpy[i] = (char*)malloc(length);
      memcpy(argv_cpy[i], argv[i], length);
    }
    argv_cpy[argc] = nullptr;

    int worldsize, provided;
    MPI_Init_thread(&argc, &argv_cpy, MPI_THREAD_SERIALIZED, &provided);
    if (provided < MPI_THREAD_SERIALIZED) {
      bdm::Log::Error(
          "", "The threading support level is lesser than that demanded.");
      MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &worldsize);

    int myrank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    if (myrank == 0) {
      auto t1 = bdm::Timing::Timestamp();
      bdm::ParallelExecutionManager pem(
          worldsize, xml_file, [&](bdm::XMLParams* params) {
            return bdm::Simulate(argc, argv, params);
          });
      int ret = pem.Start();
      MPI_Finalize();
      auto t2 = bdm::Timing::Timestamp();

      std::string result_dir = gSystem->GetWorkingDirectory();
      std::stringstream ss;
      ss << "rm " << result_dir << "/results.root";
      if (system(ss.str().c_str()) != 0) {
        bdm::Log::Warning(
            "main", "Non zero return code with with command `rm results.root`");
      }
      ss.str("");
      ss << "hadd " << result_dir << "/results.root " << "*.root > /dev/null";
      if (system(ss.str().c_str())) {
        bdm::Log::Error("main",
                        "An error occured when trying to merge .root files");
      } else {
        std::cout << "Simulation finished successfully. Results are written "
                     "to " << result_dir << "/results.root"
                  << std::endl;
      }

      std::ofstream tfile;
      tfile.open("benchmark.csv", std::ofstream::out | std::ofstream::app);
      tfile << worldsize << "," << (t2 - t1) << std::endl;
      tfile.close();

      return ret;
    } else {
      bdm::Worker w(myrank, [&](bdm::XMLParams* params) {
        return bdm::Simulate(argc, argv, params);
      });
      int ret = w.Start();
      MPI_Finalize();
      for (int i = 0; i < argc; ++i) {
        free(argv_cpy[i]);
      }
      free(argv_cpy);
      return ret;
    }
  } else {
    bdm::Log::Error("Simulate",
                    "No XML file specified in the command line argument: "
                    "--xml=/path/to/xml/file.");
    return 1;
  }
}
