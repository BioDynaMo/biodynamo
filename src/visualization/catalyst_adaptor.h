#ifndef VISUALIZATION_CATALYST_ADAPTOR_H_
#define VISUALIZATION_CATALYST_ADAPTOR_H_

#include <TError.h>
#include <string>

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
    diffusion_grid_->SetOrigin(origin_x, origin_y, origin_z);
    diffusion_grid_->SetDimensions(num_boxes[0], num_boxes[1], num_boxes[2]);
    diffusion_grid_->SetSpacing(box_length, box_length, box_length);
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

    position_array->SetName("Cell Positions");
    diameter_array->SetName("Cell Diameters");
    gradient_array->SetName("Diffusion Gradient");
    concentration_array->SetName("Substance Concentration");
    position_array->SetNumberOfComponents(3);
    gradient_array->SetNumberOfComponents(3);

    // Get diffusion grid properties
    auto rm = TResourceManager::Get();
    auto dg = rm->GetDiffusionGrids()[0];
    auto num_boxes = dg->GetNumBoxesArray();
    auto grid_dimensions = dg->GetDimensions();
    auto box_length = dg->GetBoxLength();
    auto total_boxes = num_boxes[0] * num_boxes[1] * num_boxes[2];
    auto gr_ptr = dg->GetAllGradients();
    auto co_ptr = dg->GetAllConcentrations();

    // Create the diffusion grid
    ConstructDiffusionGrid(box_length, num_boxes, grid_dimensions);

    // Point the VTK objects to our simulation data
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

    // Add attribute data to diffusion grid
    diffusion_grid_->GetPointData()->AddArray(gradient_array.GetPointer());
    diffusion_grid_->GetPointData()->AddArray(concentration_array.GetPointer());
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
      // The grid structure isn't changing so we only build it
      // the first time it's needed. If we needed the memory
      // we could delete it and rebuild as necessary.
      cells_ = vtkUnstructuredGrid::New();
      diffusion_grid_ = vtkImageData::New();
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

  /// @brief      Applies the pipeline to the data structures in the VTK Grid
  ///
  /// @param      sim_objects     The simulation objects
  /// @param[in]  step            The simulation step
  /// @param[in]  time_step       The time step duration
  /// @param[in]  last_time_step  Last time step or not
  ///
  /// @tparam     TContainer   { Container that holds the simulation objects }
  ///
  template <typename TContainer>
  inline void CoProcess(TContainer* sim_objects, double step, size_t time_step,
                        bool last_time_step) {
    vtkNew<vtkCPDataDescription> data_description;
    data_description->AddInput("cells_data");
    data_description->AddInput("diffusion_grid_data");
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
      BuildVTKDataStructures(sim_objects);
      data_description->GetInputDescriptionByName("cells_data")
          ->SetGrid(cells_);
      data_description->GetInputDescriptionByName("diffusion_grid_data")
          ->SetGrid(diffusion_grid_);
      g_processor_->CoProcess(data_description.GetPointer());
    }

    // // ----------------- User changes propagation
    // ------------------------------

    // vtkFieldData* user_data = data_description->GetUserData();
    // if (!user_data) {
    //   // no user changes
    //   return;
    // }

    // // Which properties/attribute the user changed
    // vtkStringArray* prop_arrays =
    //     vtkStringArray::SafeDownCast(user_data->GetAbstractArray("PropArrays"));
    // if (!prop_arrays) {
    //   std::cout << "Warning: Cannot find propagated array names" << endl;
    //   return;
    // }

    // // Get every changed attribute
    // vtkIdTypeArray* idx_array;
    // vtkDoubleArray* val_array;
    // for (int j = 0; j < prop_arrays->GetSize(); j++) {
    //   auto attribute = prop_arrays->GetValue(j);
    //   idx_array = vtkIdTypeArray::SafeDownCast(user_data->GetAbstractArray(
    //       (std::string("PropIdx") + std::string(attribute)).c_str()));
    //   val_array = vtkDoubleArray::SafeDownCast(user_data->GetAbstractArray(
    //       (std::string("PropVals") + std::string(attribute)).c_str()));

    //   if (!idx_array || !val_array) {
    //     std::cerr << "Warning: null pointer returned while fetching '"
    //               << attribute << "' array " << endl;
    //   }

    //   // Update changed sim_objects
    //   for (int i = 0; i < idx_array->GetNumberOfTuples(); i++) {
    //     std::cout << "sim_objects[" << idx_array->GetValue(i)
    //               << "] = " << val_array->GetValue(i) << endl;

    //     // reflection here!
    //     (*sim_objects)[idx_array->GetValue(i)].SetDiameter(
    //         val_array->GetValue(i));
    //   }
    // }
  }

 private:
  vtkCPProcessor* g_processor_ = nullptr;
  vtkImageData* diffusion_grid_ = nullptr;
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

  template <typename TContainer>
  void CoProcess(TContainer* sim_objects, double time, size_t time_step,
                 bool last_time_step) {
    Fatal("",
          "Simulation was compiled without ParaView support, but you are "
          "trying to use it.");
  }
};

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

}  // namespace bdm

#endif  // VISUALIZATION_CATALYST_ADAPTOR_H_
