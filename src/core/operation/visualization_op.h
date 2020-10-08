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

#ifndef CORE_OPERATION_VISUALIZATION_OP_H_
#define CORE_OPERATION_VISUALIZATION_OP_H_

#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/param/param.h"
#include "core/simulation.h"
#include "core/visualization/visualization_adaptor.h"

namespace bdm {

struct VisualizationOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(VisualizationOp);

  ~VisualizationOp() {
    if (visualization_) {
      delete visualization_;
    }
  }

  void Initialize() {
    auto* param = Simulation::GetActive()->GetParam();
    visualization_ = VisualizationAdaptor::Create(param->visualization_engine_);
    initialized_ = true;
  }

  void operator()() override {
    // Cannot be done in constructor because no Simulation object will be
    // available when VisualizationOp prototype gets created.
    if (!initialized_) {
      Initialize();
    }
    if (visualization_ != nullptr) {
      visualization_->Visualize();
    }
  }

  VisualizationAdaptor* visualization_ = nullptr;
  bool initialized_ = false;
};

}  // namespace bdm

#endif  // CORE_OPERATION_VISUALIZATION_OP_H_
