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

Operation::Operation() : name_("null") {}

Operation::Operation(const std::string& name, const FunctionType& f)
    : name_(name), function_(f) {}

Operation::Operation(const std::string& name, uint32_t frequency,
                     const FunctionType& f)
    : frequency_(frequency), name_(name), function_(f) {}

void Operation::operator()(SimObject* so) const { function_(so); }

}  // namespace bdm
