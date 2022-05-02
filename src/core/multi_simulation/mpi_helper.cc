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

// TODO move to core/util/mpi.cc

#include "core/multi_simulation/mpi_helper.h"

#ifdef USE_MPI

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
int MPI::init_counter_ = 0;

// -----------------------------------------------------------------------------
MPI *MPI::GetInstance() {
  static MPI kInstance;
  return &kInstance;
}

// -----------------------------------------------------------------------------
void MPI::Init(int *argc, char ***argv) {
  if (!init_counter_) {
    MPI_Init(argc, argv);
  }
  init_counter_++;
}

}  // namespace experimental
}  // namespace bdm

#endif  // USE_MPI
