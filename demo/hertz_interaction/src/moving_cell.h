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

#ifndef MOVING_CELL_H_
#define MOVING_CELL_H_

#include "biodynamo.h"
#include "core/agent/cell.h"

namespace bdm {

class Moving_cell : public Cell {
  BDM_AGENT_HEADER(Moving_cell, Cell, 1);

 public:
  Moving_cell() {}
  explicit Moving_cell(const Double3& position) : Base(position) {}
  virtual ~Moving_cell() {}

  void SetId(int id) { id_ = id; }

  int GetId() const { return id_; }

 private:
  int id_ = 0;
};

}  // namespace bdm

#endif  // MOVING_CELL_H_