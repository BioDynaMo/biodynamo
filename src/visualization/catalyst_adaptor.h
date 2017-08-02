#ifndef VISUALIZATION_CATALYST_ADAPTOR_H_
#define VISUALIZATION_CATALYST_ADAPTOR_H_

#include <iostream>
#include <string>

#ifdef USE_CATALYST

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkCPPythonScriptPipeline.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkIdTypeArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>

#endif  // USE_CATALYST

#include "cell.h"

namespace bdm {

#ifdef USE_CATALYST

class CatalystAdaptor {
 public:
  static CatalystAdaptor* GetInstance() {
    static CatalystAdaptor kInstance;
    return &kInstance;
  }

  template <typename Container>
  void BuildVTKGrid(Container* cells) {
    // create the points information
    vtkNew<vtkDoubleArray> position_array;
    vtkDoubleArray* diameter_array = vtkDoubleArray::New();
    diameter_array->SetName("Diameter");

    position_array->SetNumberOfComponents(3);
    position_array->SetArray(cells->GetPositionPtr(),
                             static_cast<vtkIdType>(cells->size() * 3), 1);
    diameter_array->SetArray(cells->GetDiameterPtr(),
                             static_cast<vtkIdType>(cells->size()), 1);

    vtkNew<vtkPoints> points;

    points->SetData(position_array.GetPointer());

    g_vtk_grid_->SetPoints(points.GetPointer());
    g_vtk_grid_->GetPointData()->AddArray(diameter_array);
  }

  template <typename Container>
  void BuildVTKDataStructures(Container* cells) {
    if (g_vtk_grid_ == NULL) {
      // The grid structure isn't changing so we only build it
      // the first time it's needed. If we needed the memory
      // we could delete it and rebuild as necessary.
      g_vtk_grid_ = vtkUnstructuredGrid::New();
      BuildVTKGrid(cells);
    }
  }

  inline void Initialize(const std::string& script) {
    if (g_processor_ == NULL) {
      g_processor_ = vtkCPProcessor::New();
      g_processor_->Initialize();
    } else {
      g_processor_->RemoveAllPipelines();
    }
    vtkNew<vtkCPPythonScriptPipeline> pipeline;
    pipeline->Initialize(script.c_str());
    g_processor_->AddPipeline(pipeline.GetPointer());
  }

  inline void Finalize() {
    if (g_processor_) {
      g_processor_->Delete();
      g_processor_ = NULL;
    }
    if (g_vtk_grid_) {
      g_vtk_grid_->Delete();
      g_vtk_grid_ = NULL;
    }
  }

  template <typename Container>
  inline void CoProcess(Container* cells, double time, size_t time_step,
                        bool last_time_step) {
    vtkNew<vtkCPDataDescription> data_description;
    data_description->AddInput("input");
    data_description->SetTimeData(time, time_step);
    if (last_time_step == true) {
      // assume that we want to all the pipelines to execute if it
      // is the last time step.
      data_description->ForceOutputOn();
    }

    // If we segfault at here it usually means that the pipeline was not
    // initialized (with a python script)
    if (g_processor_->RequestDataDescription(data_description.GetPointer()) !=
        0) {
      BuildVTKDataStructures(cells);
      data_description->GetInputDescriptionByName("input")->SetGrid(
          g_vtk_grid_);

      g_processor_->CoProcess(data_description.GetPointer());
    }

    // ----------------- User changes propagation ------------------------------

    vtkFieldData* user_data = data_description->GetUserData();
    if (!user_data) {
      // no user changes
      return;
    }

    // Which properties/attribute the user changed
    vtkStringArray* prop_arrays =
        vtkStringArray::SafeDownCast(user_data->GetAbstractArray("PropArrays"));
    if (!prop_arrays) {
      std::cout << "Warning: Cannot find propagated array names" << endl;
      return;
    }

    // Get every changed attribute
    vtkIdTypeArray* idx_array;
    vtkDoubleArray* val_array;
    for (int j = 0; j < prop_arrays->GetSize(); j++) {
      auto attribute = prop_arrays->GetValue(j);
      idx_array = vtkIdTypeArray::SafeDownCast(user_data->GetAbstractArray(
          (std::string("PropIdx") + std::string(attribute)).c_str()));
      val_array = vtkDoubleArray::SafeDownCast(user_data->GetAbstractArray(
          (std::string("PropVals") + std::string(attribute)).c_str()));

      if (!idx_array || !val_array) {
        std::cerr << "Warning: null pointer returned while fetching '"
                  << attribute << "' array " << endl;
      }

      // Update changed cells
      for (int i = 0; i < idx_array->GetNumberOfTuples(); i++) {
        std::cout << "cells[" << idx_array->GetValue(i)
                  << "] = " << val_array->GetValue(i) << endl;

        // reflection here!
        (*cells)[idx_array->GetValue(i)].SetDiameter(val_array->GetValue(i));
      }
    }
  }

 private:
  vtkCPProcessor* g_processor_ = nullptr;
  vtkUnstructuredGrid* g_vtk_grid_;
};

#else

/// False front (to ignore Catalyst in gtests)
class CatalystAdaptor {
 public:
  static CatalystAdaptor* GetInstance() {
    static CatalystAdaptor kInstance;
    return &kInstance;
  }

  template <typename Container>
  void BuildVTKGrid(Container* cells) {
    throw "Should not be called, because target not compiled with Catalyst";
  }

  template <typename Container>
  void BuildVTKDataStructures(Container* cells) {
    throw "Should not be called, because target not compiled with Catalyst";
  }

  void Initialize(const std::string& script) {
    throw "Should not be called, because target not compiled with Catalyst";
  }

  void Finalize() {
    throw "Should not be called, because target not compiled with Catalyst";
  }

  template <typename Container>
  void CoProcess(Container* cells, double time, size_t time_step,
                 bool last_time_step) {
    throw "Should not be called, because target not compiled with Catalyst";
  }
};

#endif  // USE_CATALYST

}  // namespace bdm

#endif  // VISUALIZATION_CATALYST_ADAPTOR_H_
