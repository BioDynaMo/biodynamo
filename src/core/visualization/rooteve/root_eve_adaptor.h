// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#ifndef CORE_VISUALIZATION_ROOT_EVE_ADAPTOR_H_
#define CORE_VISUALIZATION_ROOT_EVE_ADAPTOR_H_

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/visualization/visualization_adaptor.h"

namespace bdm {

/// The class that bridges the simulation code with ParaView.
class RootEveAdaptor : VisualizationAdaptor {
 public:
  /// Initializes Catalyst with the predefined pipeline and allocates memory
  /// for the VTK grid structures
  RootEveAdaptor();

  ~RootEveAdaptor();

  /// Visualize one timestep based on the configuration in `Param`
  void Visualize();

  struct RootEveImpl;

 private:
  bool initialized_ = false;  //!
  std::unique_ptr<RootEveImpl> impl_;    //!
  static std::atomic<uint64_t> counter_;  //!

  /// Parameters might be set after the constructor has been called.
  /// Therefore, we defer initialization to the first invocation of
  /// `Visualize`.
  void Initialize();

  /// Execute the insitu pipelines that were defined in `Initialize`
  void InsituVisualization();

  /// Exports the visualized objects to file, so that they can be imported and
  /// visualized in ParaView at a later point in time
  void ExportVisualization();

  /// Create the required ROOT objects to visualize agents.
  void BuildAgentsRootStructures();

  // ---------------------------------------------------------------------------
  // generate files

  void WriteSimulationInfo();
  void WritePointsTree();

  ClassDefNV(RootEveAdaptor, 1);
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_ROOT_EVE_ADAPTOR_H_
