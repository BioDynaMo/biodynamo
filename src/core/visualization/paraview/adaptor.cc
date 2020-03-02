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

#include "core/visualization/paraview/adaptor.h"
#include "core/visualization/paraview/helper.h"
#include "core/visualization/paraview/insitu_pipeline.h"

#include <cstdlib>

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
#include <vtkXMLPImageDataWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>

namespace bdm {

struct ParaviewAdaptor::ParaviewImpl {
  vtkCPProcessor* g_processor_ = nullptr;
  std::unordered_map<std::string, VtkSoGrid*> vtk_so_grids_;
  std::unordered_map<std::string, VtkDiffusionGrid*> vtk_dgrids_;
  InSituPipeline* pipeline_ = nullptr;
  vtkCPDataDescription* data_description_ = nullptr;
};

std::atomic<uint64_t> ParaviewAdaptor::counter_;

ParaviewAdaptor::ParaviewAdaptor() {
  counter_++;
  // auto* test = new ParaviewAdaptor::ParaviewImpl();
  impl_ = std::unique_ptr<ParaviewAdaptor::ParaviewImpl>(
      new ParaviewAdaptor::ParaviewImpl());
}

ParaviewAdaptor::~ParaviewAdaptor() {
  auto* param = Simulation::GetActive()->GetParam();
  counter_--;

  if (impl_) {
    if (impl_->pipeline_) {
      impl_->g_processor_->RemovePipeline(impl_->pipeline_);
      impl_->pipeline_->Delete();
      impl_->pipeline_ = nullptr;
    }

    if (counter_ == 0 && impl_->g_processor_) {
      impl_->g_processor_->RemoveAllPipelines();
      impl_->g_processor_->Finalize();
      impl_->g_processor_->Delete();
      impl_->g_processor_ = nullptr;
    }
    if (param->export_visualization_ &&
        param->visualization_export_generate_pvsm_) {
      GenerateSimulationInfoJson(impl_->vtk_so_grids_, impl_->vtk_dgrids_);
      GenerateParaviewState();
    }

    for (auto& el : impl_->vtk_so_grids_) {
      delete el.second;
    }
    for (auto& el : impl_->vtk_dgrids_) {
      delete el.second;
    }
  }
}

void ParaviewAdaptor::Visualize() {
  if (!initialized_) {
    Initialize();
    initialized_ = true;
  }

  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  uint64_t total_steps = sim->GetScheduler()->GetSimulatedSteps();
  if (total_steps % param->visualization_export_interval_ != 0) {
    return;
  }

  double time = param->simulation_time_step_ * total_steps;
  impl_->data_description_->SetTimeData(time, total_steps);

  CreateVtkObjects();

  if (param->live_visualization_ || param->python_paraview_pipeline_) {
    LiveVisualization();  // FIXME rename to InsituVisualization
  }
  if (param->export_visualization_) {
    ExportVisualization();
  }
}

void ParaviewAdaptor::Initialize() {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();

  if (param->live_visualization_ || param->export_visualization_ || param->python_paraview_pipeline_) {
    if (impl_->g_processor_ == nullptr) {
      impl_->g_processor_ = vtkCPProcessor::New();
      impl_->g_processor_->Initialize();
    }

    if (param->live_visualization_) {
        impl_->pipeline_ = new InSituPipeline();
        impl_->g_processor_->AddPipeline(impl_->pipeline_);
    } else if (param->python_paraview_pipeline_) {
      vtkNew<vtkCPPythonScriptPipeline> pipeline;
      std::string python_script =
          std::string(std::getenv("BDM_SRC_DIR")) +
          std::string("/core/visualization/paraview/simple_pipeline.py");
      pipeline->Initialize(python_script.c_str());
      impl_->g_processor_->AddPipeline(pipeline.GetPointer());
    }

    if (impl_->data_description_ == nullptr) {
      impl_->data_description_ = vtkCPDataDescription::New();
    } else {
      impl_->data_description_->Delete();
      impl_->data_description_ = vtkCPDataDescription::New();
    }
    impl_->data_description_->SetTimeData(0, 0);

    // auto* param = Simulation::GetActive()->GetParam();
    for (auto& pair : param->visualize_sim_objects_) {
      impl_->vtk_so_grids_[pair.first.c_str()] =
          new VtkSoGrid(pair.first.c_str(), impl_->data_description_);
    }
    for (auto& entry : param->visualize_diffusion_) {
      impl_->vtk_dgrids_[entry.name_] =
          new VtkDiffusionGrid(entry.name_, impl_->data_description_);
    }
  }
}

void ParaviewAdaptor::LiveVisualization() {
  impl_->g_processor_->RequestDataDescription(impl_->data_description_);
  impl_->data_description_->ForceOutputOn();
  impl_->g_processor_->CoProcess(impl_->data_description_);
}

void ParaviewAdaptor::ExportVisualization() {
  WriteToFile();
}

void ParaviewAdaptor::CreateVtkObjects() {
  BuildSimObjectsVTKStructures();
  BuildDiffusionGridVTKStructures();
}

void ParaviewAdaptor::ProcessSimObject(const SimObject* so) {
  auto* param = Simulation::GetActive()->GetParam();
  auto so_name = so->GetTypeName();

  if (param->visualize_sim_objects_.find(so_name) !=
      param->visualize_sim_objects_.end()) {

    auto* vsg = impl_->vtk_so_grids_[so->GetTypeName()];
    if (!vsg->initialized_) {
      vsg->Init(so);
    }

    ParaviewSoVisitor visitor(vsg);
    so->ForEachDataMemberIn(vsg->vis_data_members_, &visitor);
  }
}

struct ProcessSimObjectFunctor : public Functor<void, SimObject*> {
  ParaviewAdaptor* pa_;

  ProcessSimObjectFunctor(ParaviewAdaptor* pa) : pa_(pa) {}

  void operator()(SimObject* so) { pa_->ProcessSimObject(so); }
};

void ParaviewAdaptor::BuildSimObjectsVTKStructures() {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  ProcessSimObjectFunctor functor{this};
  rm->ApplyOnAllElements(functor);
}

// ---------------------------------------------------------------------------
// diffusion grids

void ParaviewAdaptor::ProcessDiffusionGrid(const DiffusionGrid* grid) {
  auto* param = Simulation::GetActive()->GetParam();
  auto name = grid->GetSubstanceName();

  // get visualization config
  const Param::VisualizeDiffusion* vd = nullptr;
  for (auto& entry : param->visualize_diffusion_) {
    if (entry.name_ == name) {
      vd = &entry;
    }
  }

  if (vd != nullptr) {
    auto* vdg = impl_->vtk_dgrids_[grid->GetSubstanceName()];
    if (!vdg->used_) {
      vdg->Init();
    }

    // If we segfault at here it probably means that there is a problem
    // with  the pipeline (either the C++ pipeline or Python pipeline)
    // We do not need to RequestDataDescription in Export Mode, because
    // we do not make use of Catalyst CoProcessing capabilities
    if (exclusive_export_viz_ ||
        (impl_->g_processor_->RequestDataDescription(
            impl_->data_description_)) != 0) {
      auto num_boxes = grid->GetNumBoxesArray();
      auto grid_dimensions = grid->GetDimensions();
      auto box_length = grid->GetBoxLength();
      auto total_boxes = grid->GetNumBoxes();

      double origin_x = grid_dimensions[0];
      double origin_y = grid_dimensions[2];
      double origin_z = grid_dimensions[4];
      vdg->data_->SetOrigin(origin_x, origin_y, origin_z);
      vdg->data_->SetDimensions(num_boxes[0], num_boxes[1], num_boxes[2]);
      vdg->data_->SetSpacing(box_length, box_length, box_length);

      if (vdg->concentration_) {
        auto* co_ptr = const_cast<double*>(grid->GetAllConcentrations());
        vdg->concentration_->SetArray(co_ptr,
                                      static_cast<vtkIdType>(total_boxes), 1);
      }
      if (vdg->gradient_) {
        auto gr_ptr = const_cast<double*>(grid->GetAllGradients());
        vdg->gradient_->SetArray(gr_ptr,
                                 static_cast<vtkIdType>(total_boxes * 3), 1);
      }
    }
  }
}

void ParaviewAdaptor::BuildDiffusionGridVTKStructures() {
  auto* rm = Simulation::GetActive()->GetResourceManager();

  rm->ApplyOnAllDiffusionGrids(
      [&](DiffusionGrid* grid) { ProcessDiffusionGrid(grid); });
}

void ParaviewAdaptor::WriteToFile() {
  auto step = impl_->data_description_->GetTimeStep();
  auto* sim = Simulation::GetActive();
  for (auto& el : impl_->vtk_so_grids_) {
    vtkNew<vtkXMLPUnstructuredGridWriter> cells_writer;
    auto filename =
        Concat(sim->GetOutputDir(), "/", el.second->name_, "-", step, ".pvtu");
    cells_writer->SetFileName(filename.c_str());
    cells_writer->SetInputData(el.second->data_);
    cells_writer->Update();
  }

  for (auto& entry : impl_->vtk_dgrids_) {
    vtkNew<vtkXMLPImageDataWriter> dgrid_writer;

    const auto& substance_name = entry.second->name_;
    auto filename =
        Concat(sim->GetOutputDir(), "/", substance_name, "-", step, ".pvti");
    dgrid_writer->SetFileName(filename.c_str());
    dgrid_writer->SetInputData(entry.second->data_);
    dgrid_writer->Update();
  }
}

/// This function generates the Paraview state based on the exported files
/// Therefore, the user can load the visualization simply by opening the pvsm
/// file and does not have to perform a lot of manual steps.
void ParaviewAdaptor::GenerateParaviewState() {
  auto* sim = Simulation::GetActive();
  std::stringstream python_cmd;
  std::string bdm_src_dir = std::getenv("BDM_SRC_DIR");

  python_cmd << bdm_src_dir << "/../third_party/paraview/bin/pvbatch "
             << bdm_src_dir
             << "/core/visualization/paraview/generate_pv_state.py "
             << sim->GetOutputDir() << "/" << kSimulationInfoJson;
  int ret_code = system(python_cmd.str().c_str());
  if (ret_code) {
    Log::Fatal("ParaviewAdaptor::GenerateParaviewState",
               "Error during generation of ParaView state\n", "Command\n",
               python_cmd.str());
  }
}

}  // namespace bdm

#else

namespace bdm {

ParaviewAdaptor::ParaviewAdaptor() {}

void ParaviewAdaptor::Visualize() {}

void ParaviewAdaptor::LiveVisualization() {}

void ParaviewAdaptor::ExportVisualization() {}

void ParaviewAdaptor::WriteToFile() {}

void ParaviewAdaptor::GenerateParaviewState() {}

}  // namespace bdm

#endif
