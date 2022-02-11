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

#include "backup_restore.h"
#ifdef BDM_USE_OMP
#include <omp.h>
#endif  // BDM_USE_OMP

int main(int argc, const char** argv) {
#ifdef BDM_USE_OMP
  omp_set_num_threads(1);
#endif  // BDM_USE_OMP
  return bdm::Simulate(argc, argv);
}
