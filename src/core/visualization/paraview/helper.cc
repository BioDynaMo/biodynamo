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

#include "core/visualization/paraview/helper.h"
#include "core/agent/agent.h"
#include "core/param/param.h"
#include "core/shape.h"
#include "core/simulation.h"

namespace bdm {

// -----------------------------------------------------------------------------
std::string GenerateSimulationInfoJson(
    const std::unordered_map<std::string, VtkAgents*>& vtk_agents,
    const std::unordered_map<std::string, VtkDiffusionGrid*>& vtk_dgrids) {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  // agents
  std::stringstream agents;
  uint64_t num_agents = param->visualize_agents.size();
  uint64_t counter = 0;
  for (const auto& entry : param->visualize_agents) {
    std::string agent_name = entry.first;

    auto search = vtk_agents.find(agent_name);
    if (search == vtk_agents.end()) {
      continue;
    }
    auto* agent_grid = search->second;

    agents << "    {" << std::endl
           << "      \"name\":\"" << agent_name << "\"," << std::endl;
    if (agent_grid->GetShape() == Shape::kSphere) {
      agents << "      \"glyph\":\"Glyph\"," << std::endl
             << "      \"shape\":\"Sphere\"," << std::endl
             << "      \"scaling_attribute\":\"diameter_\"" << std::endl;
    } else if (agent_grid->GetShape() == kCylinder) {
      agents << "      \"glyph\":\"BDMGlyph\"," << std::endl
             << "      \"shape\":\"Cylinder\"," << std::endl
             << "      \"x_scaling_attribute\":\"diameter_\"," << std::endl
             << "      \"y_scaling_attribute\":\"actual_length_\"," << std::endl
             << "      \"z_scaling_attribute\":\"diameter_\"," << std::endl
             << "      \"Vectors\":\"spring_axis_\"," << std::endl
             << "      \"MassLocation\":\"mass_location_\"" << std::endl;
    }
    agents << "    }";
    if (counter != num_agents - 1) {
      agents << ",";
    }
    agents << std::endl;
    counter++;
  }

  // extracellular substances
  std::stringstream substances;
  uint64_t num_substances = param->visualize_diffusion.size();
  bool write_comma = false;
  for (uint64_t i = 0; i < num_substances; i++) {
    auto& name = param->visualize_diffusion[i].name;

    auto search = vtk_dgrids.find(name);
    if (search == vtk_dgrids.end()) {
      continue;
    }
    auto* dgrid = search->second;
    // user wanted to export this substance, but it did not exist during
    // the entire simulation
    if (!dgrid->IsUsed()) {
      Log::Warning(
          "Visualize Diffusion Grids",
          "You are trying to visualize diffusion grid ", name,
          ", but it has not been created during the entire simulation. "
          "Please make sure the names in the "
          "configuration file match the ones in the simulation.");
      continue;
    }

    if (write_comma) {
      substances << ",\n";
    }
    substances << "    { \"name\":\"" << name << "\", ";
    std::string has_gradient =
        param->visualize_diffusion[i].gradient ? "true" : "false";
    substances << "\"has_gradient\":\"" << has_gradient << "\" }";
    write_comma = true;
  }

  std::stringstream str;
  str << "{" << std::endl
      << "  \"simulation\": {" << std::endl
      << "    \"name\":\"" << sim->GetUniqueName() << "\"," << std::endl
      << "    \"result_dir\":\"" << sim->GetOutputDir() << "\"" << std::endl
      << "  }," << std::endl
      << "  \"agents\": [" << std::endl
      << agents.str() << "  ]," << std::endl
      << "  \"extracellular_substances\": [" << std::endl
      << substances.str() << std::endl
      << "  ]," << std::endl
      << "  \"insitu_script_arguments\": \""
      << param->pv_insitu_pipelinearguments << "\"" << std::endl
      << "}" << std::endl;
  return str.str();
}

}  // namespace bdm
