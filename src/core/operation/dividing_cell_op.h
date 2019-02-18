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

#ifndef CORE_OPERATION_DIVIDING_CELL_OP_H_
#define CORE_OPERATION_DIVIDING_CELL_OP_H_

#include <cstdint>
#include "core/sim_object/cell.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"

namespace bdm {

class DividingCellOp {
 public:
  DividingCellOp() {}
  ~DividingCellOp() {}

  void operator()(SimObject* sim_object) const {
    if (Cell* cell = dynamic_cast<Cell*>(sim_object)) {
      if (cell->GetDiameter() <= 40) {
        cell->ChangeVolume(300);
      } else {
        cell->Divide();
      }
    }
  }
};

}  // namespace bdm

#endif  // CORE_OPERATION_DIVIDING_CELL_OP_H_
