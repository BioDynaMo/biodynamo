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

// check for ROOTCLING was necessary, due to ambigous reference to namespace
// detail when using ROOT I/O
#if defined(USE_PARAVIEW)

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
#include "core/visualization/paraview/so_visitor.h"
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
  std::unique_ptr<ParaviewImpl> impl_;     //!
  static std::atomic<uint64_t> counter_;  //!

  /// only needed for live visualization
  bool initialized_ = false;           //!
  bool exclusive_export_viz_ = false;  //!

  ClassDefNV(ParaviewAdaptor, 1);

  friend class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;
  friend class ParaviewAdaptorTest_GenerateParaviewState_Test;
  friend class ParaviewAdaptorTest_DISABLED_CheckVisualizationSelection_Test;
  friend class DISABLED_DiffusionTest_ModelInitializer_Test;

  /// Parameters might be set after the constructor has been called.
  /// Therefore, we defer initialization to the first invocation of
  /// `Visualize`.
  void Initialize();

  /// Applies the pipeline to the simulation objects during live visualization
  ///
  /// @param[in]  time            The simulation time
  /// @param[in]  step            The time step duration
  /// @param[in]  last_time_step  Last time step or not
  ///
  void LiveVisualization(double time, size_t step);

  /// Exports the visualized objects to file, so that they can be imported and
  /// visualized in ParaView at a later point in time
  ///
  /// @param[in]  time            The simulation time
  /// @param[in]  step            The time step
  /// @param[in]  last_time_step  The last time step
  ///
  void ExportVisualization(double time, size_t step);

  /// Creates the VTK objects that represent the simulation objects in ParaView.
  ///
  /// @param      data_description  The data description
  ///
  void CreateVtkObjects();

  // ---------------------------------------------------------------------------
  // simulation objects

  // Process a single simulation object
  void ProcessSimObject(const SimObject* so);

  /// Create the required vtk objects to visualize simulation objects.
  void BuildSimObjectsVTKStructures();

  // ---------------------------------------------------------------------------
  // diffusion grids

  /// Sets the properties of the diffusion VTK grid structures
  void ProcessDiffusionGrid(const DiffusionGrid* grid);

  /// Create the required vtk objects to visualize diffusion grids.
  void BuildDiffusionGridVTKStructures();

  // ---------------------------------------------------------------------------
  // generate files

  /// Helper function to write simulation objects to file. It loops through the
  /// vectors of VTK grid structures and calls the internal VTK writer methods
  ///
  /// @param[in]  step  The step
  ///
  void WriteToFile(size_t step);

  /// This function generates the Paraview state based on the exported files
  /// Therefore, the user can load the visualization simply by opening the pvsm
  /// file and does not have to perform a lot of manual steps.
  static void GenerateParaviewState();
};

}  // namespace bdm

#else

#include <string>
#include <unordered_map>
#include "core/shape.h"

namespace bdm {

/// False front (to ignore Catalyst in gtests)
class ParaviewAdaptor {
 public:
  ParaviewAdaptor();

  void Visualize();

 private:
  friend class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;
  friend class ParaviewAdaptorTest_GenerateParaviewState_Test;
  friend class ParaviewAdaptorTest_CheckVisualizationSelection_Test;
  friend class DISABLED_DiffusionTest_ModelInitializer_Test;

  void LiveVisualization(double time, size_t time_step);

  void ExportVisualization(double step, size_t time_step);

  void WriteToFile(size_t step);

  static void GenerateParaviewState();
};

}  // namespace bdm

#endif  // defined(USE_PARAVIEW) && !defined(__ROOTCLING__)

#endif  // CORE_VISUALIZATION_PARAVIEW_ADAPTOR_H_
