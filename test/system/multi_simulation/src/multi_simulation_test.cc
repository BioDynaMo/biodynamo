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

#include "core/multi_simulation/multi_simulation.h"
#include "multi_simulation_test.h"

using namespace bdm;

const ParamGroupUid SimParam::kUid = ParamGroupUidGenerator::Get()->NewUid();

int main(int argc, const char** argv) {
  Param::RegisterParamGroup(new SimParam());
  experimental::MultiSimulation pe(argc, argv);
  return pe.Execute(Simulate);
}
