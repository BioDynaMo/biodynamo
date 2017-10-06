#ifndef VISUALIZATION_CATALYST_ADAPTOR_H_
#define VISUALIZATION_CATALYST_ADAPTOR_H_

#include <TError.h>
#include <string>

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

  /// Build the diffusion grid visualization
  void ConstructDiffusionGrid(DiffusionGrid* dg, uint16_t idx) {
    auto num_boxes = dg->GetNumBoxesArray();
    auto grid_dimensions = dg->GetDimensions();
    auto box_length = dg->GetBoxLength();

    double origin_x = grid_dimensions[0];
    double origin_y = grid_dimensions[2];
    double origin_z = grid_dimensions[4];
    dgrids_[idx]->SetOrigin(origin_x, origin_y, origin_z);
    dgrids_[idx]->SetDimensions(num_boxes[0], num_boxes[1], num_boxes[2]);
    dgrids_[idx]->SetSpacing(box_length, box_length, box_length);
  }

  /// Builds VTK structures for given simulation object container
  ///
  /// @param      sim_objects  The simulation objects
  ///
  /// @tparam     TContainer   { Container that holds the simulation objects }
  ///
  template <typename TContainer>
  void BuildCellsVTKStructures(TContainer* sim_objects, uint16_t type_idx) {
    if (so_is_initialized_[type_idx] == false) {
      so_grids_.push_back(vtkUnstructuredGrid::New());
      so_is_initialized_[type_idx] = true;
    }

    // Build
    vtkNew<vtkDoubleArray> position_array;
    vtkNew<vtkDoubleArray> diameter_array;

    // TODO: cannot compile targets that do not have this member defined
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
    so_grids_[type_idx]->SetPoints(points.GetPointer());

    // Add attribute data to cells (i.e. cell properties)
    so_grids_[type_idx]->GetPointData()->AddArray(diameter_array.GetPointer());
    // so_grids_[type_idx]->GetPointData()->AddArray(type_array.GetPointer());
  }

  /// Builds VTK structures for given diffusion grid
  ///
  /// @param      dg    The diffusion grid
  /// @param[in]  idx   The index
  ///
  void BuildDiffusionGridVTKStructures(DiffusionGrid* dg, uint16_t idx) {
    if (dg_is_initialized_[idx] == false) {
      dgrids_.push_back(vtkImageData::New());
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
    // dgrids_[idx]->GetPointData()->AddArray(gradient_array.GetPointer());
    dgrids_[idx]->GetPointData()->AddArray(concentration_array.GetPointer());
  }

  /// @brief      Initializes Catalyst with the predefined pipeline
  ///
  /// @param[in]  script  The Python script that contains the pipeline
  ///
  inline void Initialize(const std::string& script) {
    if (g_processor_ == nullptr) {
      g_processor_ = vtkCPProcessor::New();
      g_processor_->Initialize();
      // TODO: Resize to exact numbers
      so_is_initialized_.resize(5);
      dg_is_initialized_.resize(5);
      write_ = Param::write_to_file_;
      write_freq_ = Param::write_freq_;
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
    for (auto sog : so_grids_) {
      sog->Delete();
      sog = nullptr;
    }
    for (auto dg : dgrids_) {
      dg->Delete();
      dg = nullptr;
    }
  }

  void WriteToFile(size_t step) {
    for (auto sog : so_grids_) {
      vtkNew<vtkXMLPUnstructuredGridWriter> cells_writer;

      // TODO: generate unique name for each container of cells
      std::string cells_filename =
          "cells_data_" + std::to_string(step) + ".pvtu";

      cells_writer->SetFileName(cells_filename.c_str());
      cells_writer->SetInputData(sog);
      cells_writer->Update();
    }

    int idx = 0;
    for (auto dg : dgrids_) {
      vtkNew<vtkXMLPImageDataWriter> dgrid_writer;

      // TODO: generate better unique name for each diffusion grid
      std::string dgrid_filename = "dgrid_data_" + std::to_string(idx) + "_" +
                                   std::to_string(step) + ".pvti";

      dgrid_writer->SetFileName(dgrid_filename.c_str());
      dgrid_writer->SetInputData(dg);
      dgrid_writer->Update();
      idx++;
    }
  }

  void SetWriteFrequency(double f) { write_freq_ = f; }
  void SetWrite(bool w) { write_ = w; }

  /// @brief      Applies the pipeline to the data structures in the VTK Grid
  ///
  /// @param[in]  step            The simulation step
  /// @param[in]  time_step       The time step duration
  /// @param[in]  last_time_step  Last time step or not
  ///
  inline void CoProcess(double step, size_t time_step, bool last_time_step) {
    vtkNew<vtkCPDataDescription> data_description;
    data_description->SetTimeData(step, time_step);

    // Add all simulation object containers to the visualization
    auto rm = TResourceManager::Get();
    rm->ApplyOnAllTypes([&, this](auto* sim_objects, uint16_t type_idx) {
      // TODO: Generate unique name for each container
      data_description->AddInput("cells_data");

      // If we segfault at here it probably means that the pipeline was not
      // initialized (with a python script)
      if (g_processor_->RequestDataDescription(data_description.GetPointer()) !=
          0) {
        this->BuildCellsVTKStructures(sim_objects, type_idx);
        data_description->GetInputDescriptionByName("cells_data")
            ->SetGrid(so_grids_[type_idx]);
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
            ->SetGrid(dgrids_[idx]);
      }
      idx++;
    }

    if (last_time_step == true) {
      data_description->ForceOutputOn();
    }

    g_processor_->CoProcess(data_description.GetPointer());

    if (time_step % write_freq_ == 0) {
      WriteToFile(time_step);
    }
  }

 private:
  bool write_ = false;
  size_t write_freq_ = 1;
  vtkCPProcessor* g_processor_ = nullptr;
  vector<vtkImageData*> dgrids_;
  vector<vtkUnstructuredGrid*> so_grids_;
  vector<bool> so_is_initialized_;
  vector<bool> dg_is_initialized_;
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

  void ConstructDiffusionGrid(size_t box_length,
                              const std::array<int, 3>& num_boxes,
                              const std::array<int, 6>& grid_dimensions) {
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
  }

  template <typename TContainer>
  void BuildVTKGrid(TContainer* sim_objects) {
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
  }

  template <typename TContainer>
  void BuildVTKDataStructures(TContainer* sim_objects) {
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
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
};

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

}  // namespace bdm

#endif  // VISUALIZATION_CATALYST_ADAPTOR_H_
