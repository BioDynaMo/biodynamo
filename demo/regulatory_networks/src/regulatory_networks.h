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

#ifndef DEMO_REGULATORY_NETWORKS_H_
#define DEMO_REGULATORY_NETWORKS_H_

#include <string>
#include <vector>

#include "biodynamo.h"

#include "lorenz.h"
#include "oscillator.h"
#include "sine.h"

#include "bdm_ex1.h"
#include "bdm_ex2.h"

namespace bdm {

inline int Simulate(int argc, const char** argv) {
  if (sine::Simulate(argc, argv))
    return 1;
  if (lorenz::Simulate(argc, argv))
    return 1;
  if (oscillator::Simulate(argc, argv))
    return 1;

  if (bdm::ex1::Simulate(argc, argv))
    return 1;
  if (bdm::ex2::Simulate(argc, argv))
    return 1;

  std::cout << "Simulation completed successfully!\n";
  return 0;
}

}  // namespace bdm

#endif  // DEMO_REGULATORY_NETWORKS_H_
