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

#ifndef DIVIDING_CELL_OP_H_
#define DIVIDING_CELL_OP_H_

#include <cstdint>

namespace bdm {

class DividingCellOp {
 public:
  DividingCellOp() {}
  ~DividingCellOp() {}
  DividingCellOp(const DividingCellOp&) = delete;
  DividingCellOp& operator=(const DividingCellOp&) = delete;

  template <typename TSimulation = Simulation<>>
  void operator()() const {
    auto* rm = TSimulation::GetActive()->GetResourceManager();
    rm->ApplyOnAllTypes([](auto* cells, uint16_t numa_node, uint16_t type_idx) {
#pragma omp parallel for
      for (uint64_t i = 0; i < cells->size(); i++) {
        auto&& cell = (*cells)[i];
        if (cell.GetDiameter() <= 40) {
          cell.ChangeVolume(300);
        } else {
          cell.Divide();
        }
      }
    });
  }
};

}  // namespace bdm

#endif  // DIVIDING_CELL_OP_H_
