// -----------------------------------------------------------------------------
//
// Copyright (C) 2022 CERN & University of Surrey for the benefit of the
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

#ifndef UNIT_CORE_PARAM_PARAM_TEST_H_
#define UNIT_CORE_PARAM_PARAM_TEST_H_

#include "core/param/param.h"

namespace bdm {

struct TestParamGroup : public ParamGroup {
  BDM_PARAM_GROUP_HEADER(TestParamGroup, 1);

  double test_param1 = 3.14;
  uint64_t test_param2 = 42;
  int test_param3 = -1;
};

}  // namespace bdm

#endif  // UNIT_CORE_PARAM_PARAM_TEST_H_
#include <memory>
