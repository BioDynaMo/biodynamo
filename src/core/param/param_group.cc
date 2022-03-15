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

#include "core/param/param_group.h"
#include <memory>

namespace bdm {

ParamGroupUidGenerator* ParamGroupUidGenerator::Get() {
  static ParamGroupUidGenerator kInstance;
  return &kInstance;
}

ParamGroupUidGenerator::ParamGroupUidGenerator() : counter_(0) {}

ParamGroupUid ParamGroupUidGenerator::NewUid() { return counter_++; }

ParamGroup::~ParamGroup() = default;

void ParamGroup::AssignFromConfig(const std::shared_ptr<cpptoml::table>&) {}

}  // namespace bdm
