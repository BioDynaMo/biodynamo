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

#ifndef UNIT_CORE_VISUALIZATION_PARAVIEW_ADAPTOR_TEST_H_
#define UNIT_CORE_VISUALIZATION_PARAVIEW_ADAPTOR_TEST_H_

#include "core/agent/cell.h"

namespace bdm {
namespace paraview_adaptor_test_internal {

class MyCell : public Cell {
  BDM_AGENT_HEADER(MyCell, Cell, 1);

 public:
  MyCell() {}

  int dummmy_;
};

class MyNeuron : public Cell {
  BDM_AGENT_HEADER(MyNeuron, Cell, 1);

 public:
  MyNeuron() {}

  int dummmy_;
};

}  // namespace paraview_adaptor_test_internal
}  // namespace bdm

#endif  // UNIT_CORE_VISUALIZATION_PARAVIEW_ADAPTOR_TEST_H_
