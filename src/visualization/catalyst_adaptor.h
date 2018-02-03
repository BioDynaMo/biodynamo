#ifndef VISUALIZATION_CATALYST_ADAPTOR_H_
#define VISUALIZATION_CATALYST_ADAPTOR_H_

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include "log.h"

#include "param.h"
#include "resource_manager.h"

// check for ROOTCLING was necessary, due to ambigous reference to namespace
// detail when using ROOT I/O
#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

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

#include "visualization/simple_pipeline.h"

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

namespace bdm {

#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

/// The class that bridges the simulation code with ParaView
template <typename TResourceManager = ResourceManager<>>
class CatalystAdaptor {
 public:
  static CatalystAdaptor* GetInstance() {
    static CatalystAdaptor kInstance;
    return &kInstance;
  }

  /// Sets the properties of the diffusion VTK grid structures
  ///
  /// @param      dg    The diffusion grid
  /// @param[in]  idx   Its index within the vector of diffusion VTK structures
  ///
  void ConstructDiffusionGrid(DiffusionGrid* dg, uint16_t idx) {
    auto num_boxes = dg->GetNumBoxesArray();
    auto grid_dimensions = dg->GetDimensions();
    auto box_length = dg->GetBoxLength();

    double origin_x = grid_dimensions[0];
    double origin_y = grid_dimensions[2];
    double origin_z = grid_dimensions[4];
    vtk_dgrids_[idx]->SetOrigin(origin_x, origin_y, origin_z);
    vtk_dgrids_[idx]->SetDimensions(num_boxes[0], num_boxes[1], num_boxes[2]);
    vtk_dgrids_[idx]->SetSpacing(box_length, box_length, box_length);
  }

  struct AddCellAttributeData {
    AddCellAttributeData(size_t ti, size_t nc,
                         std::vector<vtkUnstructuredGrid*>* vd)
        : type_idx(ti), num_cells(nc), vtk_data(vd) {}

    void operator()(std::vector<double>* dm, const std::string& name) {
      vtkNew<vtkDoubleArray> vtk_array;
      vtk_array->SetName(name.c_str());
      auto ptr = dm->data();
      vtk_array->SetArray(ptr, static_cast<vtkIdType>(num_cells), 1);
      (*vtk_data)[type_idx]->GetPointData()->AddArray(vtk_array.GetPointer());
    }

    void operator()(std::vector<int>* dm, const std::string& name) {
      vtkNew<vtkIntArray> vtk_array;
      vtk_array->SetName(name.c_str());
      auto ptr = dm->data();
      vtk_array->SetArray(ptr, static_cast<vtkIdType>(num_cells), 1);
      (*vtk_data)[type_idx]->GetPointData()->AddArray(vtk_array.GetPointer());
    }

    void operator()(std::vector<std::array<double, 3>>* dm,
                    const std::string& name) {
      vtkNew<vtkDoubleArray> vtk_array;
      vtk_array->SetName(name.c_str());
      auto ptr = dm->data()->data();
      vtk_array->SetNumberOfComponents(3);
      vtk_array->SetArray(ptr, static_cast<vtkIdType>(3 * num_cells), 1);
      (*vtk_data)[type_idx]->GetPointData()->AddArray(vtk_array.GetPointer());
    }

    void operator()(std::vector<std::array<int, 3>>* dm,
                    const std::string& name) {
      vtkNew<vtkIntArray> vtk_array;
      vtk_array->SetName(name.c_str());
      auto ptr = dm->data()->data();
      vtk_array->SetNumberOfComponents(3);
      vtk_array->SetArray(ptr, static_cast<vtkIdType>(3 * num_cells), 1);
      (*vtk_data)[type_idx]->GetPointData()->AddArray(vtk_array.GetPointer());
    }

    void operator()(...) {
      Log::Fatal("CatalystAdaptor::AddCellAttributeData",
                 "This data member is not supported for visualization");
    }

    size_t type_idx;
    size_t num_cells;
    std::vector<vtkUnstructuredGrid*>* vtk_data;
  };

  /// Builds the VTK grid structure for given simulation object container
  ///
  /// @param      sim_objects  The simulation objects
  ///
  /// @tparam     TContainer   { Container that holds the simulation objects }
  ///
  template <typename TContainer>
  void BuildCellsVTKStructures(TContainer* sim_objects, uint16_t type_idx) {
    if (so_is_initialized_[type_idx] == false) {
      vtk_so_grids_.push_back(vtkUnstructuredGrid::New());
      so_is_initialized_[type_idx] = true;
      shapes_.push_back(sim_objects->GetShape());
    }

    auto num_cells = sim_objects->size();
    auto& scalar_name = TContainer::GetScalarTypeName();
    auto& scalar_list = Param::visualize_sim_objects_[scalar_name];

    if (!scalar_list.empty()) {
      sim_objects->ForEachDataMemberIn(
          scalar_list,
          AddCellAttributeData(type_idx, num_cells, &vtk_so_grids_));
    }

    vtkNew<vtkDoubleArray> position_array;
    position_array->SetName("Positions");
    position_array->SetNumberOfComponents(3);
    position_array->SetArray(sim_objects->GetPositionPtr(),
                             static_cast<vtkIdType>(num_cells * 3), 1);

    // The positions of the cells need to be vtkPoints
    vtkNew<vtkPoints> points;
    points->SetData(position_array.GetPointer());
    vtk_so_grids_[type_idx]->SetPoints(points.GetPointer());
  }

  /// Builds the VTK grid structure for given diffusion grid
  ///
  /// @param      dg    The diffusion grid
  /// @param[in]  idx   The index
  ///
  void BuildDiffusionGridVTKStructures(DiffusionGrid* dg, uint16_t idx,
                                       const Param::VisualizeDiffusion& vd) {
    if (dg_is_initialized_[idx] == false) {
      vtk_dgrids_.push_back(vtkImageData::New());
      dg_is_initialized_[idx] = true;
    }

    // Create the diffusion grid
    ConstructDiffusionGrid(dg, idx);
    auto total_boxes = dg->GetNumBoxes();

    // Add attribute data
    if (vd.concentration_) {
      vtkNew<vtkDoubleArray> concentration_array;
      concentration_array->SetName("Substance Concentration");
      auto co_ptr = dg->GetAllConcentrations();
      concentration_array->SetArray(co_ptr, static_cast<vtkIdType>(total_boxes),
                                    1);
      vtk_dgrids_[idx]->GetPointData()->AddArray(
          concentration_array.GetPointer());
    }
    if (vd.gradient_) {
      vtkNew<vtkDoubleArray> gradient_array;
      gradient_array->SetName("Diffusion Gradient");
      gradient_array->SetNumberOfComponents(3);
      auto gr_ptr = dg->GetAllGradients();
      gradient_array->SetArray(gr_ptr, static_cast<vtkIdType>(total_boxes * 3),
                               1);
      vtk_dgrids_[idx]->GetPointData()->AddArray(gradient_array.GetPointer());
    }
  }

  /// Initializes Catalyst with the predefined pipeline and allocates memory
  /// for the VTK grid structures
  ///
  /// @param[in]  script  The Python script that contains the pipeline
  ///
  inline void Initialize(const std::string& script) {
    if (g_processor_ == nullptr) {
      g_processor_ = vtkCPProcessor::New();
      g_processor_->Initialize();

      auto rm = TResourceManager::Get();
      so_is_initialized_.resize(rm->NumberOfTypes());
      dg_is_initialized_.resize(rm->GetDiffusionGrids().size());
    } else {
      g_processor_->RemoveAllPipelines();
    }

    if (Param::python_catalyst_pipeline_) {
      vtkNew<vtkCPPythonScriptPipeline> pipeline;
      pipeline->Initialize(script.c_str());
      g_processor_->AddPipeline(pipeline.GetPointer());
    } else {
      pipeline_ = new vtkCPVTKPipeline();
      g_processor_->AddPipeline(pipeline_);
    }

    if (Param::export_visualization_) {
      GenerateSimulationInfoJson();
    }
  }

  /// Cleans up allocated memory
  inline void Finalize() {
    if (g_processor_) {
      g_processor_->Delete();
      g_processor_ = nullptr;
    }
    for (auto sog : vtk_so_grids_) {
      sog->Delete();
      sog = nullptr;
    }
    for (auto dg : vtk_dgrids_) {
      dg->Delete();
      dg = nullptr;
    }
    if (Param::export_visualization_) {
      GenerateParaviewState();
    }
  }

  /// Helper function to write simulation objects to file. It loops through the
  /// vectors of VTK grid structures and calls the internal VTK writer methods
  ///
  /// @param[in]  step  The step
  ///
  void WriteToFile(size_t step) {
    for (auto vtk_so : vtk_so_grids_) {
      vtkNew<vtkXMLPUnstructuredGridWriter> cells_writer;

      // TODO(ahmad): generate unique name for each container of cells
      std::string cells_filename =
          "cells_data_" + std::to_string(step) + ".pvtu";

      cells_writer->SetFileName(cells_filename.c_str());
      cells_writer->SetInputData(vtk_so);
      cells_writer->Update();
    }

    auto& dgrids = TResourceManager::Get()->GetDiffusionGrids();

    size_t idx = 0;
    for (auto vtk_dg : vtk_dgrids_) {
      vtkNew<vtkXMLPImageDataWriter> dgrid_writer;

      std::string dgrid_filename = dgrids[idx]->GetSubstanceName() + "_" +
                                   std::to_string(step) + ".pvti";

      dgrid_writer->SetFileName(dgrid_filename.c_str());
      dgrid_writer->SetInputData(vtk_dg);
      dgrid_writer->Update();
      idx++;
    }
  }

  /// Creates the VTK objects that represent the simulation objects in ParaView.
  ///
  /// @param      data_description  The data description
  ///
  void CreateVtkObjects(
      vtkNew<vtkCPDataDescription>& data_description) {  // NOLINT
    // Add simulation objects to the visualization if requested
    auto rm = TResourceManager::Get();
    rm->ApplyOnAllTypes([&, this](auto* sim_objects, uint16_t type_idx) {
      auto so_name =
          std::decay<decltype(*sim_objects)>::type::GetScalarTypeName().c_str();

      data_description->AddInput(so_name);

      // If we segfault at here it probably means that the pipeline was not
      // initialized (with a python script)
      if ((g_processor_->RequestDataDescription(
              data_description.GetPointer())) != 0) {
        this->BuildCellsVTKStructures(sim_objects, type_idx);
        data_description->GetInputDescriptionByName(so_name)->SetGrid(
            vtk_so_grids_[type_idx]);
      }
    });

    // Add all diffusion grids to the visualization if requested
    if (!Param::visualize_diffusion_.empty()) {
      uint16_t idx = 0;
      for (auto& vd : Param::visualize_diffusion_) {
        auto dg = rm->GetDiffusionGrid(vd.name_);
        if (dg == nullptr) {
          Log::Warning("Visualize Diffusion", "The substance with the name ",
                       vd.name_,
                       " was not found in the list of defined substances. "
                       "Did you spell the name correctly during "
                       "configuration?");
          continue;
        }
        data_description->AddInput(dg->GetSubstanceName().c_str());
        if (g_processor_->RequestDataDescription(
                data_description.GetPointer()) != 0) {
          this->BuildDiffusionGridVTKStructures(dg, idx, vd);
          data_description
              ->GetInputDescriptionByName(dg->GetSubstanceName().c_str())
              ->SetGrid(vtk_dgrids_[idx]);
        }
        idx++;
      }
    }
  }

  /// Applies the pipeline to the simulation objects during live visualization
  ///
  /// @param[in]  time            The simulation time
  /// @param[in]  step            The time step duration
  /// @param[in]  last_time_step  Last time step or not
  ///
  inline void CoProcess(double time, size_t step, bool last_time_step) {
    vtkNew<vtkCPDataDescription> data_description;
    data_description->SetTimeData(time, step);

    CreateVtkObjects(data_description);

    if (last_time_step == true) {
      data_description->ForceOutputOn();
    }

    if (pipeline_ != nullptr) {
      if (!(pipeline_->IsInitialized())) {
        pipeline_->Initialize(shapes_);
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

    if (step % Param::visualization_export_interval_ == 0) {
      WriteToFile(step);
    }
  }

 private:
  vtkCPProcessor* g_processor_ = nullptr;
  vtkCPVTKPipeline* pipeline_ = nullptr;
  std::vector<vtkImageData*> vtk_dgrids_;
  std::vector<vtkUnstructuredGrid*> vtk_so_grids_;
  std::vector<bool> so_is_initialized_;
  std::vector<bool> dg_is_initialized_;
  std::vector<Shape> shapes_;

  static constexpr char const* kSimulationInfoJson = "simulation_info.json";

  friend class CatalystAdaptorTest_GenerateSimulationInfoJson_Test;
  friend class CatalystAdaptorTest_GenerateParaviewState_Test;

  /// If the user selects the visualiation option export, we need to pass the
  /// information on the C++ side to a python script which generates the
  /// ParaView state file. The Json file is generated inside this function
  /// \see GenerateParaviewState
  static void GenerateSimulationInfoJson() {
    // simulation objects
    std::stringstream sim_objects;
    uint64_t num_sim_objects = Param::visualize_sim_objects_.size();
    uint64_t counter = 0;
    for (const auto& entry : Param::visualize_sim_objects_) {
      auto so_name = entry.first;
      // TODO(lukas) remove next line after export file names have been
      // generalized
      so_name = "cell";

      sim_objects << "    { \"name\":\"" << so_name << "\", ";
      // TODO(lukas) generalize
      sim_objects << "\"glyph\":\"Glyph\", \"shape\":\"Sphere\", "
                     "\"scaling_attribute\":\"diameter_\" }";
      if (counter != num_sim_objects - 1) {
        sim_objects << "," << std::endl;
      }
      counter++;
    }

    // extracellular substances
    std::stringstream substances;
    uint64_t num_substances = Param::visualize_diffusion_.size();
    for (uint64_t i = 0; i < num_substances; i++) {
      substances << "    { \"name\":\"" << Param::visualize_diffusion_[i].name_
                 << "\", ";
      std::string has_gradient =
          Param::visualize_diffusion_[i].gradient_ ? "true" : "false";
      substances << "\"has_gradient\":\"" << has_gradient << "\" }";

      if (i != num_substances - 1) {
        substances << "," << std::endl;
      }
    }

    // write to file
    std::ofstream ofstr;
    ofstr.open(kSimulationInfoJson);
    ofstr << "{" << std::endl
          << "  \"simulation\": {" << std::endl
          << "    \"name\":\"" << Param::executable_name_ << "\"," << std::endl
          << "    \"result_dir\":\""
          << "."
          << "\"" << std::endl
          << "  }," << std::endl
          << "  \"sim_objects\": [" << std::endl
          << sim_objects.str() << std::endl
          << "  ]," << std::endl
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
    std::stringstream python_cmd;
    python_cmd << "pvpython "
               << BDM_SRC_DIR "/visualization/generate_pv_state.py "
               << kSimulationInfoJson;
    int ret_code = system(python_cmd.str().c_str());
    if (ret_code) {
      Log::Fatal("CatalystAdaptor::GenerateParaviewState",
                 "Error during generation of ParaView state");
    }
  }
};

#else

/// False front (to ignore Catalyst in gtests)
template <typename TResourceManager = ResourceManager<>>
class CatalystAdaptor {
 public:
  static CatalystAdaptor* GetInstance() {
    static CatalystAdaptor kInstance;
    return &kInstance;
  }

  void Initialize(const std::string& script) {
    Log::Fatal("CatalystAdaptor::Initialize",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }

  void Finalize() {
    Log::Fatal("CatalystAdaptor::Finalize",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }

  void CoProcess(double time, size_t time_step, bool last_time_step) {
    Log::Fatal("CatalystAdaptor::CoProcess",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }

  void ExportVisualization(double step, size_t time_step, bool last_time_step) {
    Log::Fatal("CatalystAdaptor::ExportVisualization",
               "Simulation was compiled without ParaView support, but you are "
               "trying to use it.");
  }
};

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

}  // namespace bdm

#endif  // VISUALIZATION_CATALYST_ADAPTOR_H_
