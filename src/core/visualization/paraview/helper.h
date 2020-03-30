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

#ifndef CORE_VISUALIZATION_PARAVIEW_HELPER_H_
#define CORE_VISUALIZATION_PARAVIEW_HELPER_H_

// std
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
// Paraview
#include <vtkCPDataDescription.h>
#include <vtkDoubleArray.h>
#include <vtkUnstructuredGrid.h>
// BioDynaMo
#include "core/shape.h"
#include "core/visualization/paraview/vtk_diffusion_grid.h"

class TClass;

namespace bdm {

static constexpr char const* kSimulationInfoJson = "simulation_info.json";

class SimObject;

/// Adds additional data members to the `vtkUnstructuredGrid` required by
/// `ParaviewAdaptor` to visualize simulation objects.
struct VtkSoGrid {
  VtkSoGrid(const char* type_name, vtkCPDataDescription* data_description);

  ~VtkSoGrid();

  void UpdateMappedDataArrays(uint64_t tid, const std::vector<SimObject*>* sim_objects, uint64_t start, uint64_t end);

  std::string name_;
  TClass* tclass_;
  std::vector<vtkUnstructuredGrid*> data_;
  Shape shape_;

 private:
  TClass* GetTClass();
  void InitializeDataMembers(SimObject* so,
                             std::vector<std::string>* data_members);
};

// FIXME move to different file?

/// If the user selects the visualiation option export, we need to pass the
/// information on the C++ side to a python script which generates the
/// ParaView state file. The Json file is generated inside this function
/// \see GenerateParaviewState
std::string GenerateSimulationInfoJson(
    const std::unordered_map<std::string, VtkSoGrid*>& vtk_so_grids,
    const std::unordered_map<std::string, VtkDiffusionGrid*>& vtk_dgrids);

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_HELPER_H_
