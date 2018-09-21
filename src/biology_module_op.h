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

#ifndef BIOLOGY_MODULE_OP_H_
#define BIOLOGY_MODULE_OP_H_

#include "simulation_object.h"

namespace bdm {

struct BiologyModuleOp {
  void operator()(SimulationObject* sim_object) const {
    sim_object->RunBiologyModules();
  }
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_OP_H_
