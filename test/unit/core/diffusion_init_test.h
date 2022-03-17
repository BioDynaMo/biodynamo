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

#ifndef UNIT_CORE_DIFFUSION_INIT_TEST_H_
#define UNIT_CORE_DIFFUSION_INIT_TEST_H_

#include "core/diffusion/diffusion_grid.h"

namespace bdm {

// Test class for diffusion grid to
class TestGrid : public DiffusionGrid {
 public:
  TestGrid() {}
  TestGrid(int substance_id, std::string substance_name, real_t dc, real_t mu,
           int resolution = 11)
      : DiffusionGrid(substance_id, substance_name, dc, mu, resolution) {}

  void DiffuseWithClosedEdge(real_t dt) override { return; };

  void DiffuseWithOpenEdge(real_t dt) override { return; };

  void Swap() { c1_.swap(c2_); }

  // Check if the entries of c1_ and c2_ are equal for each position.
  bool CompareArrays() {
    for (size_t i = 0; i < c1_.size(); i++) {
      if (c1_[i] != c2_[i]) {
        return false;
      }
    }
    return true;
  }

  // Compares all values of the array c1_ with a specific value.
  bool ComapareArrayWithValue(real_t value) {
    for (size_t i = 0; i < c1_.size(); i++) {
      if (c1_[i] != value) {
        return false;
      }
    }
    return true;
  }

 private:
  BDM_CLASS_DEF_OVERRIDE(TestGrid, 1);
};

}  // namespace bdm

#endif  // UNIT_CORE_DIFFUSION_INIT_TEST_H_
