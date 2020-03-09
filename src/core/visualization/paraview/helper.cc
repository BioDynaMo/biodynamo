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

#include "core/visualization/paraview/helper.h"
#include "core/visualization/paraview/jit.h"
#include "core/param/param.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"


#include "core/param/param.h"
#include "core/shape.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/visualization/paraview/jit.h"

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>

namespace bdm {

VtkSoGrid::VtkSoGrid(const char* type_name, vtkCPDataDescription* data_description) {
  data_ = vtkUnstructuredGrid::New();
  name_ = type_name;
  data_description->AddInput(type_name);
  data_description->GetInputDescriptionByName(type_name)->SetGrid(data_);

  InitializeVtkSoGrid(this);
}

VtkSoGrid::~VtkSoGrid() {
  name_ = "";
  data_->Delete();
  data_ = nullptr;
  vis_data_members_.clear();
}

void VtkSoGrid::ResetAndResizeDataArrays(uint64_t new_size) {
  auto* parray = data_->GetPoints()->GetData();
  parray->Reset();
  parray->SetNumberOfTuples(new_size);

  auto* point_data = data_->GetPointData();
  for (int i = 0; i < point_data->GetNumberOfArrays(); i++) {
    auto* array = point_data->GetArray(i);
    array->Reset();
    array->SetNumberOfTuples(new_size);
  }
}

// -----------------------------------------------------------------------------
VtkDiffusionGrid::VtkDiffusionGrid(const std::string& name,
                 vtkCPDataDescription* data_description) {
  data_ = vtkImageData::New();
  name_ = name;

  // get visualization config
  auto* param = Simulation::GetActive()->GetParam();
  const Param::VisualizeDiffusion* vd = nullptr;
  for (auto& entry : param->visualize_diffusion_) {
    if (entry.name_ == name) {
      vd = &entry;
      break;
    }
  }

  // Add attribute data
  if (vd->concentration_) {
    vtkNew<vtkDoubleArray> concentration;
    concentration->SetName("Substance Concentration");
    concentration_ = concentration.GetPointer();
    data_->GetPointData()->AddArray(concentration.GetPointer());
  }
  if (vd->gradient_) {
    vtkNew<vtkDoubleArray> gradient;
    gradient->SetName("Diffusion Gradient");
    gradient->SetNumberOfComponents(3);
    gradient_ = gradient.GetPointer();
    data_->GetPointData()->AddArray(gradient.GetPointer());
  }

  data_description->AddInput(name.c_str());
  data_description->GetInputDescriptionByName(name.c_str())->SetGrid(data_);
}

VtkDiffusionGrid::~VtkDiffusionGrid() {
  name_ = "";
  data_->Delete();
  data_ = nullptr;
  concentration_ = nullptr;
  gradient_ = nullptr;
}

void VtkDiffusionGrid::Init() { used_ = true; }

// -----------------------------------------------------------------------------
std::string GenerateSimulationInfoJson(
    const std::unordered_map<std::string, VtkSoGrid*>& vtk_so_grids,
    const std::unordered_map<std::string, VtkDiffusionGrid*>& vtk_dgrids) {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  // simulation objects
  std::stringstream sim_objects;
  uint64_t num_sim_objects = param->visualize_sim_objects_.size();
  uint64_t counter = 0;
  for (const auto& entry : param->visualize_sim_objects_) {
    std::string so_name = entry.first;

    auto search = vtk_so_grids.find(so_name);
    if (search == vtk_so_grids.end()) {
      continue;
    }
    auto* so_grid = search->second;

    sim_objects << "    {" << std::endl
                << "      \"name\":\"" << so_name << "\"," << std::endl;
    if (so_grid->shape_ == Shape::kSphere) {
      sim_objects << "      \"glyph\":\"Glyph\"," << std::endl
                  << "      \"shape\":\"Sphere\"," << std::endl
                  << "      \"scaling_attribute\":\"diameter_\"" << std::endl;
    } else if (so_grid->shape_ == kCylinder) {
      sim_objects << "      \"glyph\":\"BDMGlyph\"," << std::endl
                  << "      \"shape\":\"Cylinder\"," << std::endl
                  << "      \"x_scaling_attribute\":\"diameter_\"," << std::endl
                  << "      \"y_scaling_attribute\":\"actual_length_\","
                  << std::endl
                  << "      \"z_scaling_attribute\":\"diameter_\"," << std::endl
                  << "      \"Vectors\":\"spring_axis_\"," << std::endl
                  << "      \"MassLocation\":\"mass_location_\"" << std::endl;
    }
    sim_objects << "    }";
    if (counter != num_sim_objects - 1) {
      sim_objects << ",";
    }
    sim_objects << std::endl;
    counter++;
  }

  // extracellular substances
  std::stringstream substances;
  uint64_t num_substances = param->visualize_diffusion_.size();
  for (uint64_t i = 0; i < num_substances; i++) {
    auto& name = param->visualize_diffusion_[i].name_;

    auto search = vtk_dgrids.find(name);
    if (search == vtk_dgrids.end()) {
      continue;
    }
    auto* dgrid = search->second;
    // user wanted to export this substance, but it did not exist during
    // the entire simulation
    if (!dgrid->used_) {
      Log::Warning(
          "Visualize Diffusion Grids",
          "You are trying to visualize diffusion grid ", name,
          ", but it has not been created during the entire simulation. "
          "Please make sure the names in the "
          "configuration file match the ones in the simulation.");
      num_substances--;
      continue;
    }
    substances << "    { \"name\":\"" << name << "\", ";
    std::string has_gradient =
        param->visualize_diffusion_[i].gradient_ ? "true" : "false";
    substances << "\"has_gradient\":\"" << has_gradient << "\" }";

    if (i != num_substances - 1) {
      substances << "," << std::endl;
    }
  }

  // write to file
  std::stringstream str;
  str << "{" << std::endl
      << "  \"simulation\": {" << std::endl
      << "    \"name\":\"" << sim->GetUniqueName() << "\"," << std::endl
      << "    \"result_dir\":\"" << sim->GetOutputDir() << "\"" << std::endl
      << "  }," << std::endl
      << "  \"sim_objects\": [" << std::endl
      << sim_objects.str() << "  ]," << std::endl
      << "  \"extracellular_substances\": [" << std::endl
      << substances.str() << std::endl
      << "  ]" << std::endl
      << "}" << std::endl;
  return str.str();
}

}  // namespace bdm
