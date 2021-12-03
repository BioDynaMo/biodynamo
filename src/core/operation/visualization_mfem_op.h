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

#ifdef USE_MFEM
#ifndef CORE_OPERATION_VISUALIZATION_MFEM_OP_H_
#define CORE_OPERATION_VISUALIZATION_MFEM_OP_H_

#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "mfem.hpp"

namespace bdm {

class VisualizationMFEMOp : public StandaloneOperationImpl {
  BDM_OP_HEADER(VisualizationMFEMOp);

 public:
  virtual ~VisualizationMFEMOp();

  /// Creates one ParaViewDataCollection for each Mfem Grid (mesh + scalar
  /// field) to write out the data.
  void Initialize();

  /// Saves the state of the MFEM continua to a paraview file. When called for
  /// the first time, it automatically calls Initialize().
  void operator()() override;

 private:
  /// Vector to keep track of different ParaViewDataCollection(s), e.g. one for
  /// each MFEM grid.
  std::vector<mfem::ParaViewDataCollection*> paraview_data_;
  /// False if VisualizationMFEMOp::Initialize() has not been called yet. Used
  /// to only initialize once.
  bool initialized_ = false;
  /// InsituVisualization are not supported. To only warn once, we use this
  /// parameter.
  bool has_warned_ = false;
};

}  // namespace bdm

#endif  // CORE_OPERATION_VISUALIZATION_MFEM_OP_H_
#endif  // USE_MFEM