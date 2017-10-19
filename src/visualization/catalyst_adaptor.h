#ifndef VISUALIZATION_CATALYST_ADAPTOR_H_
#define VISUALIZATION_CATALYST_ADAPTOR_H_

#include <TError.h>
#include <string>
#include <vector>

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
    }

    // Build
    vtkNew<vtkDoubleArray> position_array;
    vtkNew<vtkDoubleArray> diameter_array;

    // TODO(ahmad): cannot compile targets that do not have this member defined
    // vtkNew<vtkIntArray> type_array;

    // The names that will appear in ParaView
    // type_array->SetName("Type");
    position_array->SetName("Cell Positions");
    diameter_array->SetName("Cell Diameters");

    // position has three components (x, y, z)
    position_array->SetNumberOfComponents(3);

    // Point the VTK objects to our simulation data
    // type_array->SetArray(sim_objects->GetCellTypePtr(),
    // static_cast<vtkIdType>(sim_objects->size()), 1);
    position_array->SetArray(sim_objects->GetPositionPtr(),
                             static_cast<vtkIdType>(sim_objects->size() * 3),
                             1);
    diameter_array->SetArray(sim_objects->GetDiameterPtr(),
                             static_cast<vtkIdType>(sim_objects->size()), 1);

    // The positions of the cells need to be vtkPoints
    vtkNew<vtkPoints> points;
    points->SetData(position_array.GetPointer());
    vtk_so_grids_[type_idx]->SetPoints(points.GetPointer());

    // Add attribute data to cells (i.e. cell properties)
    vtk_so_grids_[type_idx]->GetPointData()->AddArray(
        diameter_array.GetPointer());
    // vtk_so_grids_[type_idx]->GetPointData()->AddArray(type_array.GetPointer());
  }

  /// Builds the VTK grid structure for given diffusion grid
  ///
  /// @param      dg    The diffusion grid
  /// @param[in]  idx   The index
  ///
  void BuildDiffusionGridVTKStructures(DiffusionGrid* dg, uint16_t idx) {
    if (dg_is_initialized_[idx] == false) {
      vtk_dgrids_.push_back(vtkImageData::New());
      dg_is_initialized_[idx] = true;
    }

    // vtkNew<vtkDoubleArray> gradient_array;
    vtkNew<vtkDoubleArray> concentration_array;

    // gradient_array->SetName("Diffusion Gradient");
    concentration_array->SetName("Substance Concentration");

    // Gradient has three components (x, y, z)
    // gradient_array->SetNumberOfComponents(3);

    // auto gr_ptr = dg->GetAllGradients();
    auto co_ptr = dg->GetAllConcentrations();

    // Create the diffusion grid
    ConstructDiffusionGrid(dg, idx);

    auto total_boxes = dg->GetNumBoxes();
    // gradient_array->SetArray(gr_ptr, static_cast<vtkIdType>(total_boxes * 3),
    // 1);
    concentration_array->SetArray(co_ptr, static_cast<vtkIdType>(total_boxes),
                                  1);

    // Add attribute data to diffusion grid
    // vtk_dgrids_[idx]->GetPointData()->AddArray(gradient_array.GetPointer());
    vtk_dgrids_[idx]->GetPointData()->AddArray(
        concentration_array.GetPointer());
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
    vtkNew<vtkCPPythonScriptPipeline> pipeline;
    pipeline->Initialize(script.c_str());
    g_processor_->AddPipeline(pipeline.GetPointer());
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
    // Add all simulation object containers to the visualization
    auto rm = TResourceManager::Get();
    rm->ApplyOnAllTypes([&, this](auto* sim_objects, uint16_t type_idx) {
      // TODO(ahmad): Generate unique name for each container
      data_description->AddInput("cells_data");

      // If we segfault at here it probably means that the pipeline was not
      // initialized (with a python script)
      if (g_processor_->RequestDataDescription(data_description.GetPointer()) !=
          0) {
        this->BuildCellsVTKStructures(sim_objects, type_idx);
        data_description->GetInputDescriptionByName("cells_data")
            ->SetGrid(vtk_so_grids_[type_idx]);
      }
    });

    // Add all diffusion grids to the visualization
    auto& dgs = rm->GetDiffusionGrids();
    uint16_t idx = 0;
    for (auto dg : dgs) {
      data_description->AddInput(dg->GetSubstanceName().c_str());
      if (g_processor_->RequestDataDescription(data_description.GetPointer()) !=
          0) {
        this->BuildDiffusionGridVTKStructures(dg, idx);
        data_description
            ->GetInputDescriptionByName(dg->GetSubstanceName().c_str())
            ->SetGrid(vtk_dgrids_[idx]);
      }
      idx++;
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

    if (step % Param::write_freq_ == 0) {
      WriteToFile(step);
    }
  }

 private:
  vtkCPProcessor* g_processor_ = nullptr;
  std::vector<vtkImageData*> vtk_dgrids_;
  std::vector<vtkUnstructuredGrid*> vtk_so_grids_;
  std::vector<bool> so_is_initialized_;
  std::vector<bool> dg_is_initialized_;
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
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
  }

  void Finalize() {
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
  }

  void CoProcess(double time, size_t time_step, bool last_time_step) {
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
  }

  void ExportVisualization(double step, size_t time_step, bool last_time_step) {
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
  }
};

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

}  // namespace bdm

#endif  // VISUALIZATION_CATALYST_ADAPTOR_H_
