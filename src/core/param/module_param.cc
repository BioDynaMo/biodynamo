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

#include "core/param/module_param.h"
#include <memory>

namespace bdm {

ModuleParamUidGenerator* ModuleParamUidGenerator::Get() {
  static ModuleParamUidGenerator kInstance;
  return &kInstance;
}

ModuleParamUidGenerator::ModuleParamUidGenerator() : counter_(0) {}

ModuleParamUid ModuleParamUidGenerator::NewUid() { return counter_++; }

ModuleParam::~ModuleParam() {}

void ModuleParam::AssignFromConfig(const std::shared_ptr<cpptoml::table>&) {}

}  // namespace bdm
