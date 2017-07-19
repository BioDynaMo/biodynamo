#include <iostream>

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

#include "bdmAdaptor.h"

namespace {
vtkCPProcessor* gProcessor = NULL;
vtkUnstructuredGrid* gVTKGrid;

void BuildVTKGrid(Cell<Soa>* cells) {
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

  gVTKGrid->SetPoints(points.GetPointer());
  gVTKGrid->GetPointData()->AddArray(diameter_array);
}

void BuildVTKDataStructures(Cell<Soa>* cells) {
  if (gVTKGrid == NULL) {
    // The grid structure isn't changing so we only build it
    // the first time it's needed. If we needed the memory
    // we could delete it and rebuild as necessary.
    gVTKGrid = vtkUnstructuredGrid::New();
    BuildVTKGrid(cells);
  }
}
}  // namespace

namespace bdm_adaptor {

void Initialize(char* script) {
  if (gProcessor == NULL) {
    gProcessor = vtkCPProcessor::New();
    gProcessor->Initialize();
  } else {
    gProcessor->RemoveAllPipelines();
  }
  vtkNew<vtkCPPythonScriptPipeline> pipeline;
  pipeline->Initialize(script);
  gProcessor->AddPipeline(pipeline.GetPointer());
}

void Finalize() {
  if (gProcessor) {
    gProcessor->Delete();
    gProcessor = NULL;
  }
  if (gVTKGrid) {
    gVTKGrid->Delete();
    gVTKGrid = NULL;
  }
}

void CoProcess(Cell<Soa>* cells, double time, size_t time_step,
               bool last_time_step) {
  vtkNew<vtkCPDataDescription> data_description;
  data_description->AddInput("input");
  data_description->SetTimeData(time, time_step);
  if (last_time_step == true) {
    // assume that we want to all the pipelines to execute if it
    // is the last time step.
    data_description->ForceOutputOn();
  }
  if (gProcessor->RequestDataDescription(data_description.GetPointer()) != 0) {
    BuildVTKDataStructures(cells);
    data_description->GetInputDescriptionByName("input")->SetGrid(gVTKGrid);

    std::cout << "[" << time_step << "] Starting CoProcess..." << endl;
    gProcessor->CoProcess(data_description.GetPointer());
    std::cout << "[" << time_step << "] Done!" << endl;
  }

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
}  // namespace bdm_adaptor
