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

#include "mpi.h"

int main(int argc, const char** argv) {
  // We need to pass a non-const version of argv to MPI_Init, so we make a deep
  // copy
  char** argv_cpy = (char**)malloc((argc + 1) * sizeof *argv_cpy);
  for (int i = 0; i < argc; ++i) {
    size_t length = strlen(argv[i]) + 1;
    argv_cpy[i] = (char*)malloc(length);
    memcpy(argv_cpy[i], argv[i], length);
  }
  auto options = bdm::CommandLineOptions(argc, argv);
  std::string xml_file = options.Get<std::string>("xml");
  if (xml_file != "") {
    int myrank, worldsize;
    MPI_Init(&argc, &argv_cpy);
    MPI_Comm_size(MPI_COMM_WORLD, &worldsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    if (worldsize == 1) {  // Single-Process Execution
      // TODO(ahmad): Take into account that single-process execution
      // could still be dealing with multiple parameter sets
      return bdm::Simulate(argc, argv);
    } else {  // Multi-Process Execution
      if (myrank == 0) {
        bdm::ParallelExecutionManager pem(worldsize, xml_file);
        int ret = pem.Start();
        MPI_Finalize();
        return ret;
      } else {
        bdm::Worker w(myrank, [&](bdm::XMLParams* params) {
          return bdm::Simulate(argc, argv, params);
        });
        int ret = w.Start();
        MPI_Finalize();
        return ret;
      }
    }
  } else {
    bdm::Log::Error("Simulate",
               "No XML file specified in the command line argument: "
               "--xml=/path/to/xml/file.");
    return bdm::Simulate(argc, argv);
  }
}
