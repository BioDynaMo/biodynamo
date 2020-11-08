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

#include "neuroscience/module.h"
#include "core/param/param.h"
#include "neuroscience/param.h"

namespace bdm {
namespace neuroscience {

void InitModule() {
  using NeuroscienceParam = bdm::neuroscience::Param;
  bdm::Param::RegisterParamGroup(new NeuroscienceParam());
}

}  // namespace neuroscience
}  // namespace bdm
