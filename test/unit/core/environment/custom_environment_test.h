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

#ifndef TEST_UNIT_CORE_ENVIRONMENT_CUSTOM_ENVIRONMENT_TEST_H_
#define TEST_UNIT_CORE_ENVIRONMENT_CUSTOM_ENVIRONMENT_TEST_H_

#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "gtest/gtest.h"
#include "unit/test_util/test_util.h"

#include <string>

namespace bdm {

// Needs to be in a separate header to be included in dictionary generation
class APerson : public Cell {
  BDM_AGENT_HEADER(APerson, Cell, 1);

 public:
  APerson() {}
  explicit APerson(int a, bool g, std::string c)
      : age_(a), gender_(g), city_(c) {}
  virtual ~APerson() {}

  int age_ = 0;
  bool gender_ = 0;
  std::string city_ = "";
};

}  // namespace bdm

#endif  // TEST_UNIT_CORE_ENVIRONMENT_CUSTOM_ENVIRONMENT_TEST_H_
