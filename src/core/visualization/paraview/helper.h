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
#include <string>
#include <unordered_map>
// BioDynaMo
#include "core/shape.h"
#include "core/visualization/paraview/vtk_diffusion_grid.h"
#include "core/visualization/paraview/vtk_agents.h"

class TClass;

namespace bdm {

static constexpr char const* kSimulationInfoJson = "simulation_info.json";

/// If the user selects the visualiation option export, we need to pass the
/// information on the C++ side to a python script which generates the
/// ParaView state file. The Json file is generated inside this function
/// \see GenerateParaviewState
std::string GenerateSimulationInfoJson(
    const std::unordered_map<std::string, VtkAgents*>& vtk_agents,
    const std::unordered_map<std::string, VtkDiffusionGrid*>& vtk_dgrids);

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_HELPER_H_
