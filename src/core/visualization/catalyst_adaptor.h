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

#ifndef CORE_VISUALIZATION_CATALYST_ADAPTOR_H_
#define CORE_VISUALIZATION_CATALYST_ADAPTOR_H_

// check for ROOTCLING was necessary, due to ambigous reference to namespace
// detail when using ROOT I/O
#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/scheduler.h"
#include "core/util/log.h"
#include "core/visualization/insitu_pipeline.h"
#include "core/visualization/catalyst_helper_structs.h"
#include "core/visualization/catalyst_so_visitor.h"

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

/// The class that bridges the simulation code with ParaView.
/// Requires that simulation objects use the Soa memory layout.
class CatalystAdaptor {
 public:
  /// Initializes Catalyst with the predefined pipeline and allocates memory
  /// for the VTK grid structures
  ///
  /// @param[in]  script  The Python script that contains the pipeline
  ///
  explicit CatalystAdaptor(const std::string& script) : python_script_(script) {
    counter_++;
  }

  ~CatalystAdaptor() {
    auto* param = Simulation::GetActive()->GetParam();
    counter_--;

    if (pipeline_) {
      g_processor_->RemovePipeline(pipeline_);
      pipeline_->Delete();
      pipeline_ = nullptr;
    }

    if (counter_ == 0 && g_processor_) {
      g_processor_->RemoveAllPipelines();
      g_processor_->Finalize();
      g_processor_->Delete();
      g_processor_ = nullptr;
    }
    if (param->export_visualization_) {
      GenerateSimulationInfoJson(vtk_so_grids_, vtk_dgrids_);
      GenerateParaviewState();
    }

    for(auto& el : vtk_so_grids_) {
      delete el.second;
    }
    for(auto& el : vtk_dgrids_) {
      delete el.second;
    }
  }

  /// Visualize one timestep based on the configuration in `Param`
  void Visualize(uint64_t total_steps, bool last_iteration) {
    if (!initialized_) {
      Initialize();
      initialized_ = true;
    }

    auto* param = Simulation::GetActive()->GetParam();
    if (total_steps % param->visualization_export_interval_ != 0) {
      return;
    }

    if (param->live_visualization_) {
      double time = param->simulation_time_step_ * total_steps;
      LiveVisualization(time, total_steps, last_iteration);
    }
    if (param->export_visualization_) {
      double time = param->simulation_time_step_ * total_steps;
      ExportVisualization(time, total_steps, last_iteration);
    }
  }

 private:
  static vtkCPProcessor* g_processor_;
  static std::atomic<uint64_t> counter_;

  /// only needed for live visualization
  InSituPipeline* pipeline_ = nullptr;
  std::string python_script_;
  bool initialized_ = false;
  bool exclusive_export_viz_ = false;
  std::unordered_map<std::string, VtkSoGrid*> vtk_so_grids_;
  std::unordered_map<std::string, VtkDiffusionGrid*> vtk_dgrids_;

  static constexpr char const* kSimulationInfoJson = "simulation_info.json";

  friend class CatalystAdaptorTest_GenerateSimulationInfoJson_Test;
  friend class CatalystAdaptorTest_GenerateParaviewState_Test;
  friend class CatalystAdaptorTest_CheckVisualizationSelection_Test;
  friend class DISABLED_DiffusionTest_ModelInitializer_Test;

  /// Parameters might be set after the constructor has been called.
  /// Therefore, we defer initialization to the first invocation of
  /// `Visualize`.
  void Initialize() {
    auto* sim = Simulation::GetActive();
    auto* param = sim->GetParam();

    exclusive_export_viz_ =
        param->export_visualization_ && !param->live_visualization_;
    if (param->live_visualization_ || param->export_visualization_) {
      if (g_processor_ == nullptr) {
        g_processor_ = vtkCPProcessor::New();
        g_processor_->Initialize();
      }

      if (param->live_visualization_ &&
          g_processor_->GetNumberOfPipelines() != 0) {
        Log::Fatal("CatalystAdaptor",
                   "Live visualization does not support multiple "
                   "simulations. Turning off live visualization for ",
                   sim->GetUniqueName());
      } else if (param->python_catalyst_pipeline_) {
        vtkNew<vtkCPPythonScriptPipeline> pipeline;
        pipeline->Initialize(python_script_.c_str());
        g_processor_->AddPipeline(pipeline.GetPointer());
      } else if (!exclusive_export_viz_) {
        pipeline_ = new InSituPipeline();
        g_processor_->AddPipeline(pipeline_);
      }

      vtkNew<vtkCPDataDescription> data_description;
      data_description->SetTimeData(0, 0);

      auto* param = Simulation::GetActive()->GetParam();
      for(auto& pair : param->visualize_sim_objects_) {
        vtk_so_grids_[pair.first.c_str()] = new VtkSoGrid(pair.first.c_str(), data_description);
      }
      for(auto& entry : param->visualize_diffusion_) {
        vtk_dgrids_[entry.name_] = new VtkDiffusionGrid(entry.name_, data_description);
      }
    }
  }

  /// Applies the pipeline to the simulation objects during live visualization
  ///
  /// @param[in]  time            The simulation time
  /// @param[in]  step            The time step duration
  /// @param[in]  last_time_step  Last time step or not
  ///
  void LiveVisualization(double time, size_t step, bool last_time_step) {
    vtkNew<vtkCPDataDescription> data_description;
    data_description->SetTimeData(time, step);

    CreateVtkObjects(data_description);

    if (last_time_step == true) {
      data_description->ForceOutputOn();
    }

    if (pipeline_ != nullptr) {
      if (!(pipeline_->IsInitialized())) {
        pipeline_->Initialize(vtk_so_grids_);
      }
    }

    g_processor_->CoProcess(data_description.GetPointer());
  }

  /// Exports the visualized objects to file, so that they can be imported and
  /// visualized in ParaView at a later point in time
  ///
  /// @param[in]  time            The simulation time
  /// @param[in]  step            The time step
  /// @param[in]  last_time_step  The last time step
  ///
  inline void ExportVisualization(double time, size_t step,
                                  bool last_time_step) {
    vtkNew<vtkCPDataDescription> data_description;
    data_description->SetTimeData(time, step);

    CreateVtkObjects(data_description);

    if (last_time_step == true) {
      data_description->ForceOutputOn();
    }

    WriteToFile(step);
  }

  /// Creates the VTK objects that represent the simulation objects in ParaView.
  ///
  /// @param      data_description  The data description
  ///
  void CreateVtkObjects(vtkNew<vtkCPDataDescription>& data_description) {  // NOLINT
    BuildSimObjectsVTKStructures(data_description);
    BuildDiffusionGridVTKStructures(data_description);
  }

  // ---------------------------------------------------------------------------
  // simulation objects

  // Process a single simulation object
  void ProcessSimObject(const SimObject* so, vtkNew<vtkCPDataDescription>& data_description) {
    auto* param = Simulation::GetActive()->GetParam();
    auto so_name = so->GetTypeName();

    if (param->visualize_sim_objects_.find(so_name) !=
        param->visualize_sim_objects_.end()) {

      // assert(vtk_so_grids_.find(so->GetTypeName()) != vtk_so_grids_.end() && Concat("VtkSoGrid for ", so->GetTypeName(), " not found!");
      auto* vsg = vtk_so_grids_[so->GetTypeName()];
      if (!vsg->initialized_) {
        vsg->Init(so);
      }

      // If we segfault at here it probably means that there is a problem
      // with the pipeline (either the C++ pipeline or Python pipeline)
      // We do not need to RequestDataDescription in Export Mode, because
      // we do not make use of Catalyst CoProcessing capabilities
      if (exclusive_export_viz_ ||
          (g_processor_->RequestDataDescription(
              data_description.GetPointer())) != 0) {
         CatalystSoVisitor visitor(vsg);
         so->ForEachDataMemberIn(vsg->vis_data_members_, &visitor);
      }
    }
  }

  /// Create the required vtk objects to visualize simulation objects.
  void BuildSimObjectsVTKStructures(vtkNew<vtkCPDataDescription>& data_description) {
    auto* rm = Simulation::GetActive()->GetResourceManager();

    rm->ApplyOnAllElements([&](SimObject* so ){
      ProcessSimObject(so, data_description);
    });
  }

  // ---------------------------------------------------------------------------
  // diffusion grids

  /// Sets the properties of the diffusion VTK grid structures
  void ProcessDiffusionGrid(const DiffusionGrid* grid, vtkNew<vtkCPDataDescription>& data_description) {
    auto* param = Simulation::GetActive()->GetParam();
    auto name = grid->GetSubstanceName();

    // get visualization config
    const Param::VisualizeDiffusion* vd = nullptr;
    for(auto& entry : param->visualize_diffusion_) {
      if (entry.name_ == name) {
        vd = &entry;
      }
    }

    if (vd != nullptr) {
      auto* vdg = vtk_dgrids_[grid->GetSubstanceName()];
      if (!vdg->used_) {
        vdg->Init();
      }

      // If we segfault at here it probably means that there is a problem
      // with  the pipeline (either the C++ pipeline or Python pipeline)
      // We do not need to RequestDataDescription in Export Mode, because
      // we do not make use of Catalyst CoProcessing capabilities
      if (exclusive_export_viz_ ||
          (g_processor_->RequestDataDescription(
              data_description.GetPointer())) != 0) {

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

        if(vdg->concentration_) {
          auto* co_ptr = const_cast<double*>(grid->GetAllConcentrations());
          vdg->concentration_->SetArray(co_ptr, static_cast<vtkIdType>(total_boxes), 1);
        }
        if(vdg->gradient_) {
          auto gr_ptr = const_cast<double*>(grid->GetAllGradients());
          vdg->gradient_->SetArray(gr_ptr, static_cast<vtkIdType>(total_boxes * 3), 1);
        }
      }
    }
  }

  /// Create the required vtk objects to visualize diffusion grids.
  void BuildDiffusionGridVTKStructures(vtkNew<vtkCPDataDescription>& data_description) {
    auto* rm = Simulation::GetActive()->GetResourceManager();

    rm->ApplyOnAllDiffusionGrids([&](DiffusionGrid* grid ){
      ProcessDiffusionGrid(grid, data_description);
    });
  }

  // ---------------------------------------------------------------------------
  // generate files

  /// Helper function to write simulation objects to file. It loops through the
  /// vectors of VTK grid structures and calls the internal VTK writer methods
  ///
  /// @param[in]  step  The step
  ///
  void WriteToFile(size_t step) {
    auto* sim = Simulation::GetActive();
    for (auto& el : vtk_so_grids_) {
      vtkNew<vtkXMLPUnstructuredGridWriter> cells_writer;
      auto filename =
          Concat(sim->GetOutputDir(), "/", el.second->name_, "-", step, ".pvtu");
      cells_writer->SetFileName(filename.c_str());
      cells_writer->SetInputData(el.second->data_);
      cells_writer->Update();
    }

    for (auto& entry : vtk_dgrids_) {
      vtkNew<vtkXMLPImageDataWriter> dgrid_writer;

      const auto& substance_name = entry.second->name_;
      auto filename =
          Concat(sim->GetOutputDir(), "/", substance_name, "-", step, ".pvti");
      dgrid_writer->SetFileName(filename.c_str());
      dgrid_writer->SetInputData(entry.second->data_);
      dgrid_writer->Update();
    }
  }

  /// If the user selects the visualiation option export, we need to pass the
  /// information on the C++ side to a python script which generates the
  /// ParaView state file. The Json file is generated inside this function
  /// \see GenerateParaviewState
  static void GenerateSimulationInfoJson(
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
      // user wanted to export this simulation object type, but not a single
      // instance has been created during the entire simulation
      if (!so_grid->initialized_) {
        Log::Warning("Visualize Simulation Objects",
                 "You are trying to visualize simulation object type ", so_name,
                 ", but not a single instance has been created during the "
                 "entire simulation. Please make sure the names in the "
                 "configuration file match the ones in the simulation.");
        num_sim_objects--;
        continue;
      }

      sim_objects << "    {" << std::endl
                  << "      \"name\":\"" << so_name << "\"," << std::endl;
      if (so_grid->shape_ == Shape::kSphere) {
        sim_objects << "      \"glyph\":\"Glyph\"," << std::endl
                    << "      \"shape\":\"Sphere\"," << std::endl
                    << "      \"scaling_attribute\":\"diameter_\"" << std::endl;
      } else if (so_grid->shape_ == kCylinder) {
        sim_objects << "      \"glyph\":\"BDMGlyph\"," << std::endl
                    << "      \"shape\":\"Cylinder\"," << std::endl
                    << "      \"x_scaling_attribute\":\"diameter_\","
                    << std::endl
                    << "      \"y_scaling_attribute\":\"actual_length_\","
                    << std::endl
                    << "      \"z_scaling_attribute\":\"diameter_\","
                    << std::endl
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
      if(!dgrid->used_) {
        Log::Warning("Visualize Diffusion Grids",
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
    std::ofstream ofstr;
    ofstr.open(Concat(sim->GetOutputDir(), "/", kSimulationInfoJson));
    ofstr << "{" << std::endl
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
    ofstr.close();
  }

  /// This function generates the Paraview state based on the exported files
  /// Therefore, the user can load the visualization simply by opening the pvsm
  /// file and does not have to perform a lot of manual steps.
  static void GenerateParaviewState() {
    auto* sim = Simulation::GetActive();
    std::stringstream python_cmd;
    python_cmd << "pvpython "
               << BDM_SRC_DIR "/core/visualization/generate_pv_state.py "
               << sim->GetOutputDir() << "/" << kSimulationInfoJson;
    int ret_code = system(python_cmd.str().c_str());
    if (ret_code) {
      Log::Fatal("CatalystAdaptor::GenerateParaviewState",
                 "Error during generation of ParaView state\n", "Command\n",
                 python_cmd.str());
    }
  }
};

}  // namespace bdm

#else

#include <string>
#include <unordered_map>
#include "core/shape.h"

namespace bdm {

struct VtkSoGrid;
struct VtkDiffusionGrid;

/// False front (to ignore Catalyst in gtests)
class CatalystAdaptor {
 public:
  explicit CatalystAdaptor(const std::string& script) {}

  void Visualize(uint64_t, bool) {}

 private:
  friend class CatalystAdaptorTest_GenerateSimulationInfoJson_Test;
  friend class CatalystAdaptorTest_GenerateParaviewState_Test;
  friend class CatalystAdaptorTest_CheckVisualizationSelection_Test;
  friend class DISABLED_DiffusionTest_ModelInitializer_Test;

  void LiveVisualization(double time, size_t time_step, bool last_time_step) {}

  void ExportVisualization(double step, size_t time_step, bool last_time_step) {
  }

  void WriteToFile(size_t step) {}

  static void GenerateSimulationInfoJson(
      const std::unordered_map<std::string, VtkSoGrid*>& vtk_so_grids,
      const std::unordered_map<std::string, VtkDiffusionGrid*>& vtk_dgrids) {}

  static void GenerateParaviewState() {}
};

}  // namespace bdm

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

#endif  // CORE_VISUALIZATION_CATALYST_ADAPTOR_H_
