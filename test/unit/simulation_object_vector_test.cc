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

#include "simulation_object_vector_test.h"

// FIXME investigate: without default_ctparam -> incomplete type error
#include "default_ctparam.h"
#include "bdm_imp.h"

namespace bdm {
namespace simulation_object_vector_test_internal {

TEST(SimulationObjectVector, All) { RunTest(); }

TEST(SimulationObjectVector, InitializeSuccessors) { RunInitializeTest(); }

}  // namespace simulation_object_vector_test_internal
}  // namespace bdm
