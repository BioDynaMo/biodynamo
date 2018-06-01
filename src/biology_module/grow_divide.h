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

#ifndef BIOLOGY_MODULE_GROW_DIVIDE_H_
#define BIOLOGY_MODULE_GROW_DIVIDE_H_

#include <Rtypes.h>
#include "biology_module_util.h"

namespace bdm {

/// This biology module grows the simulation object until the diameter reaches
/// the specified threshold and divides the object afterwards.
struct GrowDivide : public BaseBiologyModule {
  GrowDivide() : BaseBiologyModule(gAllBmEvents) {}
  GrowDivide(double threshold, double growth_rate,
             std::initializer_list<BmEvent> event_list)
      : BaseBiologyModule(event_list),
        threshold_(threshold),
        growth_rate_(growth_rate) {}

  template <typename T>
  void Run(T* cell) {
    if (cell->GetDiameter() <= threshold_) {
      cell->ChangeVolume(growth_rate_);
    } else {
      cell->Divide();
    }
  }

 private:
  double threshold_ = 40;
  double growth_rate_ = 300;
  ClassDefNV(GrowDivide, 1);
};

}  // namespace bdm

#endif  // BIOLOGY_MODULE_GROW_DIVIDE_H_
