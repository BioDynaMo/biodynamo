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
#include <vtkXMLPImageDataWriter.h>
#include <vtkXMLPUnstructuredGridWriter.h>
#include <vtkUnstructuredGrid.h>

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
  void ConstructDiffusionGrid(size_t box_length,
                              const std::array<int, 3>& num_boxes,
                              const std::array<int, 6>& grid_dimensions) {
    double origin_x = grid_dimensions[0];
    double origin_y = grid_dimensions[2];
    double origin_z = grid_dimensions[4];
    dgrid_->SetOrigin(origin_x, origin_y, origin_z);
    dgrid_->SetDimensions(num_boxes[0], num_boxes[1], num_boxes[2]);
    dgrid_->SetSpacing(box_length, box_length, box_length);
  }

  /// @brief      Builds a VTK grid.
  ///
  /// @param      sim_objects  The simulation objects
  ///
  /// @tparam     TContainer   { Container that holds the simulation objects }
  ///
  template <typename TContainer>
  void BuildVTKGrid(TContainer* sim_objects) {
    // Prepare VTK objects
    vtkNew<vtkDoubleArray> position_array;
    vtkNew<vtkDoubleArray> diameter_array;
    vtkNew<vtkDoubleArray> gradient_array;
    vtkNew<vtkDoubleArray> concentration_array;
    vtkNew<vtkIntArray> type_array;

    type_array->SetName("Type");
    position_array->SetName("Cell Positions");
    diameter_array->SetName("Cell Diameters");
    gradient_array->SetName("Diffusion Gradient");
    concentration_array->SetName("Substance Concentration");
    position_array->SetNumberOfComponents(3);
    gradient_array->SetNumberOfComponents(3);

    // Get diffusion grid properties
    auto rm = TResourceManager::Get();
    auto& dg = rm->GetDiffusionGrids()[1];
    auto num_boxes = dg->GetNumBoxesArray();
    auto grid_dimensions = dg->GetDimensions();
    auto box_length = dg->GetBoxLength();
    auto total_boxes = num_boxes[0] * num_boxes[1] * num_boxes[2];
    auto gr_ptr = dg->GetAllGradients();
    auto co_ptr = dg->GetAllConcentrations();

    // Create the diffusion grid
    ConstructDiffusionGrid(box_length, num_boxes, grid_dimensions);

    // Point the VTK objects to our simulation data
    type_array->SetArray(sim_objects->GetCellTypePtr(),
                         static_cast<vtkIdType>(sim_objects->size()), 1);
    position_array->SetArray(sim_objects->GetPositionPtr(),
                             static_cast<vtkIdType>(sim_objects->size() * 3),
                             1);
    diameter_array->SetArray(sim_objects->GetDiameterPtr(),
                             static_cast<vtkIdType>(sim_objects->size()), 1);
    gradient_array->SetArray(gr_ptr, static_cast<vtkIdType>(total_boxes * 3),
                             1);
    concentration_array->SetArray(co_ptr, static_cast<vtkIdType>(total_boxes),
                                  1);

    // Add attribute data to cells
    vtkNew<vtkPoints> points;
    points->SetData(position_array.GetPointer());
    cells_->SetPoints(points.GetPointer());
    cells_->GetPointData()->AddArray(diameter_array.GetPointer());
    cells_->GetPointData()->AddArray(type_array.GetPointer());

    // Add attribute data to diffusion grid
    dgrid_->GetPointData()->AddArray(gradient_array.GetPointer());
    dgrid_->GetPointData()->AddArray(concentration_array.GetPointer());
  }

  /// @brief      Wrapper around @ref BuildVTKGRid to define
  ///
  /// @param      sim_objects  The simulation objects
  ///
  /// @tparam     TContainer   { Container that holds the simulation objects }
  ///
  template <typename TContainer>
  void BuildVTKDataStructures(TContainer* sim_objects) {
    if (cells_ == nullptr) {
      write_ = Param::write_to_file_;
      write_freq_ = Param::write_freq_;
      // The grid structure isn't changing so we only build it
      // the first time it's needed. If we needed the memory
      // we could delete it and rebuild as necessary.
      cells_ = vtkUnstructuredGrid::New();
      dgrid_ = vtkImageData::New();
    }
    BuildVTKGrid(sim_objects);
  }

  /// @brief      Initializes Catalyst with the predefined pipeline
  ///
  /// @param[in]  script  The Python script that contains the pipeline
  ///
  inline void Initialize(const std::string& script) {
    if (g_processor_ == nullptr) {
      g_processor_ = vtkCPProcessor::New();
      g_processor_->Initialize();
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
      // Call made to MPI_FINALIZE
      g_processor_->Delete();
      g_processor_ = nullptr;
    }
    if (cells_) {
      cells_->Delete();
      cells_ = nullptr;
    }
  }

  void WriteToFile(size_t step) {
    vtkNew<vtkXMLPUnstructuredGridWriter> cells_writer;
    vtkNew<vtkXMLPImageDataWriter> dgrid_writer;

    std::string cells_filename = "cells_data_" + std::to_string(step) + ".pvtu";
    std::string dgrid_filename = "dgrid_data_" + std::to_string(step) + ".pvti";

    cells_writer->SetFileName(cells_filename.c_str());
    dgrid_writer->SetFileName(dgrid_filename.c_str());
    cells_writer->SetInputData(cells_);
    dgrid_writer->SetInputData(dgrid_);

    cells_writer->Update();
    dgrid_writer->Update();
  }

  void SetWriteFrequency(double f) { write_freq_ = f; }
  void SetWrite(bool w) { write_ = w; }

  /// @brief      Applies the pipeline to the data structures in the VTK Grid
  ///
  /// @param[in]  step            The simulation step
  /// @param[in]  time_step       The time step duration
  /// @param[in]  last_time_step  Last time step or not
  ///
  inline void CoProcess(double step, size_t time_step,
                        bool last_time_step) {
    auto rm = TResourceManager::Get();
    rm->ApplyOnAllTypes([&,this](auto* sim_objects, uint16_t type_idx) {
      vtkNew<vtkCPDataDescription> data_description;
      data_description->AddInput("cells_data");
      data_description->AddInput("dgrid_data");
      data_description->SetTimeData(step, time_step);
      if (last_time_step == true) {
        // assume that we want to all the pipelines to execute if it
        // is the last time step.
        data_description->ForceOutputOn();
      }

      // If we segfault at here it probably means that the pipeline was not
      // initialized (with a python script)
      if (g_processor_->RequestDataDescription(data_description.GetPointer()) !=
          0) {
        this->BuildVTKDataStructures(sim_objects);
        data_description->GetInputDescriptionByName("cells_data")
            ->SetGrid(cells_);
        data_description->GetInputDescriptionByName("dgrid_data")
            ->SetGrid(dgrid_);
        g_processor_->CoProcess(data_description.GetPointer());
      }
    });

    if (time_step % write_freq_ == 0) {
      WriteToFile(time_step);
    }
  }

 private:
  bool write_ = false;
  size_t write_freq_ = 1;
  vtkCPProcessor* g_processor_ = nullptr;
  vtkImageData* dgrid_ = nullptr;
  vtkUnstructuredGrid* cells_ = nullptr;
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
