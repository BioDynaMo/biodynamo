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

#include "core/operation/operation.h"
#include "core/sim_object/sim_object.h"

namespace bdm {

Operation::Operation(const std::string& name) : name_(name) {}

Operation::Operation(const std::string& name, uint32_t frequency)
    : frequency_(frequency), name_(name) {}

Operation::~Operation() {}

}  // namespace bdm
