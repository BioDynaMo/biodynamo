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
vtkCPProcessor* Processor = NULL;
vtkUnstructuredGrid* VTKGrid;

void BuildVTKGrid(Cell<Soa>& cells) {
  // create the points information
  vtkNew<vtkDoubleArray> positionArray;
  vtkDoubleArray* diameterArray = vtkDoubleArray::New();
  diameterArray->SetName("Diameter");

  positionArray->SetNumberOfComponents(3);
  positionArray->SetArray(cells.GetPositionPtr(),
                          static_cast<vtkIdType>(cells.size() * 3), 1);
  diameterArray->SetArray(cells.GetDiameterPtr(),
                          static_cast<vtkIdType>(cells.size()), 1);

  vtkNew<vtkPoints> points;

  points->SetData(positionArray.GetPointer());

  VTKGrid->SetPoints(points.GetPointer());
  VTKGrid->GetPointData()->AddArray(diameterArray);
}

void BuildVTKDataStructures(Cell<Soa>& cells) {
  if (VTKGrid == NULL) {
    // The grid structure isn't changing so we only build it
    // the first time it's needed. If we needed the memory
    // we could delete it and rebuild as necessary.
    VTKGrid = vtkUnstructuredGrid::New();
    BuildVTKGrid(cells);
  }
}
}  // namespace

namespace bdmAdaptor {

void Initialize(char* script) {
  if (Processor == NULL) {
    Processor = vtkCPProcessor::New();
    Processor->Initialize();
  } else {
    Processor->RemoveAllPipelines();
  }
  vtkNew<vtkCPPythonScriptPipeline> pipeline;
  pipeline->Initialize(script);
  Processor->AddPipeline(pipeline.GetPointer());
}

void Finalize() {
  if (Processor) {
    Processor->Delete();
    Processor = NULL;
  }
  if (VTKGrid) {
    VTKGrid->Delete();
    VTKGrid = NULL;
  }
}

void CoProcess(Cell<Soa>& cells, double time, size_t timeStep,
               bool lastTimeStep) {
  vtkNew<vtkCPDataDescription> dataDescription;
  dataDescription->AddInput("input");
  dataDescription->SetTimeData(time, timeStep);
  if (lastTimeStep == true) {
    // assume that we want to all the pipelines to execute if it
    // is the last time step.
    dataDescription->ForceOutputOn();
  }
  if (Processor->RequestDataDescription(dataDescription.GetPointer()) != 0) {
    BuildVTKDataStructures(cells);
    dataDescription->GetInputDescriptionByName("input")->SetGrid(VTKGrid);
    
    std::cout << "[" << timeStep << "] Starting CoProcess..." << endl;
    Processor->CoProcess(dataDescription.GetPointer());
    std::cout << "[" << timeStep << "] Done!" << endl;
  }
  
  vtkFieldData *userData = dataDescription->GetUserData();
  if (!userData) {
    // no user changes
    return;
  }
    
  // Which properties/attribute the user changed
  vtkStringArray *propArrays = vtkStringArray::SafeDownCast(
          userData->GetAbstractArray("PropArrays"));
  if (!propArrays) {
    std::cout << "Warning: Cannot find propagated array names" << endl;
    return;
  }

  // Get every changed attribute
  vtkIdTypeArray *idxArray;
  vtkDoubleArray *valArray;
  for (int j = 0; j < propArrays->GetSize(); j++) {
    auto attribute = propArrays->GetValue(j);
    idxArray = vtkIdTypeArray::SafeDownCast( userData->GetAbstractArray(
        (std::string("PropIdx") + std::string(attribute)).c_str()
    ));
    valArray = vtkDoubleArray::SafeDownCast( userData->GetAbstractArray(
        (std::string("PropVals") + std::string(attribute)).c_str()
    ));

    if (!idxArray || !valArray) {
        std::cerr << "Warning: null pointer returned while fetching '"  
            << attribute << "' array " << endl;
    }
    
    // Update changed cells
    for (int i = 0; i < idxArray->GetNumberOfTuples(); i++) {
      std::cout << "cells[" << idxArray->GetValue(i) << "] = " \
          << valArray->GetValue(i) << endl;
      
      // reflection here!
      cells[ idxArray->GetValue(i) ].SetDiameter( valArray->GetValue(i) );
    }
  }

}
}  // namespace bdmAdaptor
