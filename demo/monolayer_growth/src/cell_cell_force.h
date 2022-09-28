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

#ifndef CELL_CELL_FORCE_H_
#define CELL_CELL_FORCE_H_

#include "biodynamo.h"
#include "core/interaction_force.h"

namespace bdm {

class CellCellForce : public InteractionForce {
 public:
  CellCellForce() {}
  virtual ~CellCellForce() {}

  virtual Real4 Calculate(const Agent* lhs, const Agent* rhs) const override;
};

}  // namespace bdm

#endif
