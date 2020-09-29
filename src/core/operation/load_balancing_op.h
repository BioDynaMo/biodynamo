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

#ifndef CORE_OPERATION_LOAD_BALANCING_OP_H_
#define CORE_OPERATION_LOAD_BALANCING_OP_H_

#include "core/operation/operation.h"
#include "core/resource_manager.h"
#include "core/simulation.h"

namespace bdm {

/// A class that sets up diffusion grids of the substances in this simulation
struct LoadBalancingOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(LoadBalancingOp);

  void operator()() override {
    auto* sim = Simulation::GetActive();
    auto* rm = sim->GetResourceManager();
    rm->SortAndBalanceNumaNodes();
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_LOAD_BALANCING_OP_H_
