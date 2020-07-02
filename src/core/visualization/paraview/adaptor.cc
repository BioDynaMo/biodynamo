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

#include <cstdlib>
#include <fstream>
#include <sstream>

#include "core/visualization/paraview/adaptor.h"
#include "core/visualization/paraview/helper.h"
#include "core/visualization/paraview/vtk_diffusion_grid.h"
#include "core/visualization/paraview/vtk_sim_objects.h"

#ifndef __ROOTCLING__

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkCPPythonScriptPipeline.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkIdTypeArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLPImageDataWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkImageDataStreamer.h>

namespace bdm {

// ----------------------------------------------------------------------------
struct ParaviewAdaptor::ParaviewImpl {
  vtkCPProcessor* g_processor_ = nullptr;
  std::unordered_map<std::string, VtkSimObjects*> vtk_sim_objects_;
  std::unordered_map<std::string, VtkDiffusionGrid*> vtk_dgrids_;
  vtkCPDataDescription* data_description_ = nullptr;
};

// ----------------------------------------------------------------------------
std::atomic<uint64_t> ParaviewAdaptor::counter_;

// ----------------------------------------------------------------------------
ParaviewAdaptor::ParaviewAdaptor() {
  counter_++;
  impl_ = std::unique_ptr<ParaviewAdaptor::ParaviewImpl>(
      new ParaviewAdaptor::ParaviewImpl());
}

// ----------------------------------------------------------------------------
ParaviewAdaptor::~ParaviewAdaptor() {
  auto* param = Simulation::GetActive()->GetParam();
  counter_--;

  if (impl_) {
    if (counter_ == 0 && impl_->g_processor_) {
      impl_->g_processor_->RemoveAllPipelines();
      impl_->g_processor_->Finalize();
      impl_->g_processor_->Delete();
      impl_->g_processor_ = nullptr;
    }
    if (param->export_visualization_ &&
        param->visualization_export_generate_pvsm_) {
      WriteSimulationInfoJsonFile();
      GenerateParaviewState();
    }

    if (impl_->data_description_ != nullptr) {
      impl_->data_description_->Delete();
    }
    for (auto& el : impl_->vtk_sim_objects_) {
      delete el.second;
    }
    for (auto& el : impl_->vtk_dgrids_) {
      delete el.second;
    }
  }
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::Visualize() {
  if (!initialized_) {
    Initialize();
    initialized_ = true;
  }

  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  uint64_t total_steps = sim->GetScheduler()->GetSimulatedSteps();
  if (total_steps % param->visualization_interval != 0) {
    return;
  }

  double time = param->simulation_time_step_ * total_steps;
  impl_->data_description_->SetTimeData(time, total_steps);

  CreateVtkObjects();

  if (param->insitu_visualization_) {
    InsituVisualization();
  }
  if (param->export_visualization_) {
    ExportVisualization();
  }
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::Initialize() {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();

  if (param->insitu_visualization_ && impl_->g_processor_ == nullptr) {
    impl_->g_processor_ = vtkCPProcessor::New();
    impl_->g_processor_->Initialize();
  }

  if (param->insitu_visualization_) {
    const std::string& script = ParaviewAdaptor::BuildPythonScriptString(
        param->pv_insitu_pipeline_);
    std::ofstream ofs;
    auto* sim = Simulation::GetActive();
    // TODO(lukas) use vtkCPPythonStringPipeline once we update to Paraview
    // v5.8
    std::string final_python_script_name =
        Concat(sim->GetOutputDir(), "/insitu_pipline.py");
    ofs.open(final_python_script_name);
    ofs << script;
    ofs.close();
    vtkNew<vtkCPPythonScriptPipeline> pipeline;
    pipeline->Initialize(final_python_script_name.c_str());
    impl_->g_processor_->AddPipeline(pipeline.GetPointer());
  }

  if (impl_->data_description_ == nullptr) {
    impl_->data_description_ = vtkCPDataDescription::New();
  } else {
    impl_->data_description_->Delete();
    impl_->data_description_ = vtkCPDataDescription::New();
  }
  impl_->data_description_->SetTimeData(0, 0);

  for (auto& pair : param->visualize_sim_objects_) {
    impl_->vtk_sim_objects_[pair.first.c_str()] =
        new VtkSimObjects(pair.first.c_str(), impl_->data_description_);
  }
  for (auto& entry : param->visualize_diffusion_) {
    impl_->vtk_dgrids_[entry.name_] =
        new VtkDiffusionGrid(entry.name_, impl_->data_description_);
  }
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::InsituVisualization() {
#ifdef BDM_PV_EXPERIMENTAL
  vtkCommand::kOpenGLCacheEnable = true;
  vtkCommand::kOpenGLCacheIndex = 0;
#endif  // BDM_PV_EXPERIMENTAL
  impl_->g_processor_->RequestDataDescription(impl_->data_description_);
  impl_->data_description_->ForceOutputOn();
  impl_->g_processor_->CoProcess(impl_->data_description_);
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::ExportVisualization() { 
   WriteSimulationInfoJsonFile();

   auto step = impl_->data_description_->GetTimeStep();

   for (auto& el : impl_->vtk_sim_objects_) {
     el.second->WriteToFile(step);
   }

   for (auto& el : impl_->vtk_dgrids_) {
     el.second->WriteToFile(step);
   }
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::CreateVtkObjects() {
  BuildSimObjectsVTKStructures();
  BuildDiffusionGridVTKStructures();
  if (impl_->data_description_->GetUserData() == nullptr) {
    std::cout << "Create user data " << std::endl;
    vtkNew<vtkStringArray> json;
    json->SetName("metadata");
    json->InsertNextValue(
        GenerateSimulationInfoJson(impl_->vtk_sim_objects_, impl_->vtk_dgrids_));
    vtkNew<vtkFieldData> field;
    field->AddArray(json);
    impl_->data_description_->SetUserData(field);
  }
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::BuildSimObjectsVTKStructures() {
  auto* rm = Simulation::GetActive()->GetResourceManager();
  for (auto& pair : impl_->vtk_sim_objects_) {
    const auto& sim_objects = rm->GetTypeIndex()->GetType(pair.second->GetTClass());

    std::cout << "sim object size " << sim_objects.size() << std::endl;
    pair.second->Update(&sim_objects);
  }
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::BuildDiffusionGridVTKStructures() {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  rm->ApplyOnAllDiffusionGrids(
      [&](DiffusionGrid* grid) {
    auto it = impl_->vtk_dgrids_.find(grid->GetSubstanceName());
    if (it != impl_->vtk_dgrids_.end()) {
      it->second->Update(grid); 
    }
  });
}

// ----------------------------------------------------------------------------
void ParaviewAdaptor::WriteSimulationInfoJsonFile() {
  // create simulation_info.json 
  if (!simulation_info_json_generated_) {
    std::ofstream ofstr;
    auto* sim = Simulation::GetActive();
    ofstr.open(Concat(sim->GetOutputDir(), "/", kSimulationInfoJson));
    ofstr << GenerateSimulationInfoJson(impl_->vtk_sim_objects_,
                                        impl_->vtk_dgrids_);
    ofstr.close();
    simulation_info_json_generated_ = true;
  }
}

// ----------------------------------------------------------------------------
/// This function generates the Paraview state based on the exported files
/// Therefore, the user can load the visualization simply by opening the pvsm
/// file and does not have to perform a lot of manual steps.
void ParaviewAdaptor::GenerateParaviewState() {
  auto* sim = Simulation::GetActive();
  std::stringstream python_cmd;
  std::string pv_dir = std::getenv("ParaView_DIR");
  std::string bdmsys = std::getenv("BDMSYS");

  python_cmd << pv_dir << "/bin/pvbatch "
             << bdmsys
             << "/include/core/visualization/paraview/generate_pv_state.py "
             << sim->GetOutputDir() << "/" << kSimulationInfoJson;
  int ret_code = system(python_cmd.str().c_str());
  if (ret_code) {
    Log::Fatal("ParaviewAdaptor::GenerateParaviewState",
               "Error during generation of ParaView state\n", "Command\n",
               python_cmd.str());
  }
}

// ----------------------------------------------------------------------------
std::string ParaviewAdaptor::BuildPythonScriptString(
    const std::string& python_script) {
  std::stringstream script;

  std::ifstream ifs;
  ifs.open(python_script, std::ifstream::in);
  if (!ifs.is_open()) {
    Log::Fatal("ParaviewAdaptor::BuildPythonScriptString",
               Concat("Python script (", python_script,
                      ") was not found or could not be opened."));
  }
  script << ifs.rdbuf();
  ifs.close();

  std::string default_python_script =
      std::string(std::getenv("BDMSYS")) +
      std::string("/include/core/visualization/paraview/default_insitu_pipeline.py");

  std::ifstream ifs_default;
  ifs_default.open(default_python_script, std::ifstream::in);
  if (!ifs_default.is_open()) {
    Log::Fatal("ParaviewAdaptor::BuildPythonScriptString",
               Concat("Python script (", default_python_script,
                      ") was not found or could not be opened."));
  }
  script << std::endl << ifs_default.rdbuf();
  ifs_default.close();
  return script.str();
}

}  // namespace bdm

#else

namespace bdm {

ParaviewAdaptor::ParaviewAdaptor() {}

void ParaviewAdaptor::Visualize() {}

void ParaviewAdaptor::InsituVisualization() {}

void ParaviewAdaptor::ExportVisualization() {}

void ParaviewAdaptor::WriteToFile() {}

void ParaviewAdaptor::GenerateParaviewState() {}

std::string ParaviewAdaptor::BuildPythonScriptString(
    const std::string& python_script) {}
}  // namespace bdm

#endif
