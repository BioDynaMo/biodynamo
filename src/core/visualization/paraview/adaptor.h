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

#ifndef CORE_VISUALIZATION_PARAVIEW_ADAPTOR_H_
#define CORE_VISUALIZATION_PARAVIEW_ADAPTOR_H_

#include <algorithm>
#include <cstdlib>
#include <fstream>
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
class ParaviewAdaptor : VisualizationAdaptor {
 public:
  /// Initializes Catalyst with the predefined pipeline and allocates memory
  /// for the VTK grid structures
  ParaviewAdaptor();

  ~ParaviewAdaptor();

  /// Visualize one timestep based on the configuration in `Param`
  void Visualize();

  struct ParaviewImpl;

 private:
  std::unique_ptr<ParaviewImpl> impl_;    //!
  static std::atomic<uint64_t> counter_;  //!

  /// only needed for insitu visualization
  bool initialized_ = false;  //!
  bool simulation_info_json_generated_ = false;

  friend class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;
  friend class ParaviewAdaptorTest_GenerateParaviewState_Test;
  friend class ParaviewAdaptorTest_DISABLED_CheckVisualizationSelection_Test;
  friend class DISABLED_DiffusionTest_ModelInitializer_Test;

  /// Parameters might be set after the constructor has been called.
  /// Therefore, we defer initialization to the first invocation of
  /// `Visualize`.
  void Initialize();

  /// Execute the insitu pipelines that were defined in `Initialize`
  void InsituVisualization();

  /// Exports the visualized objects to file, so that they can be imported and
  /// visualized in ParaView at a later point in time
  void ExportVisualization();

  /// Creates the VTK objects that represent the simulation objects in ParaView.
  void CreateVtkObjects();

  /// Create the required vtk objects to visualize simulation objects.
  void BuildSimObjectsVTKStructures();

  /// Create the required vtk objects to visualize diffusion grids.
  void BuildDiffusionGridVTKStructures();

  // ---------------------------------------------------------------------------
  // generate files

  void WriteSimulationInfoJsonFile();

  /// This function generates the Paraview state based on the exported files
  /// Therefore, the user can load the visualization simply by opening the pvsm
  /// file and does not have to perform a lot of manual steps.
  static void GenerateParaviewState();

  /// Combine user-defined python script with biodynamo default python
  /// insitu pipeline.
  static std::string BuildPythonScriptString(const std::string& python_script);

  ClassDefNV(ParaviewAdaptor, 1);
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_ADAPTOR_H_
