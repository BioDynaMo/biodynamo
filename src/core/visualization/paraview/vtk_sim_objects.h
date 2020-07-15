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

#ifndef CORE_VISUALIZATION_PARAVIEW_VTK_SIM_OBJECTS_H_
#define CORE_VISUALIZATION_PARAVIEW_VTK_SIM_OBJECTS_H_

// std
#include <string>
#include <vector>
// Paraview
#include <vtkCPDataDescription.h>
#include <vtkUnstructuredGrid.h>
// BioDynaMo
#include "core/shape.h"
#include "core/sim_object/sim_object.h"

class TClass;

namespace bdm {

class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;

/// Adds additional data members to the `vtkUnstructuredGrid` required by
/// `ParaviewAdaptor` to visualize simulation objects.
class VtkSimObjects {
 public:
  VtkSimObjects(const char* type_name, vtkCPDataDescription* data_description);

  ~VtkSimObjects();

  vtkUnstructuredGrid* GetData(uint64_t idx);
  Shape GetShape() const;
  TClass* GetTClass();
  void Update(const std::vector<SimObject*>* sim_objects);
  void WriteToFile(uint64_t step) const;

 private:
  std::string name_;
  TClass* tclass_;
  std::vector<vtkUnstructuredGrid*> data_;
  Shape shape_;

  TClass* FindTClass();
  void InitializeDataMembers(SimObject* so,
                             std::vector<std::string>* data_members);
  void UpdateMappedDataArrays(uint64_t tid,
                              const std::vector<SimObject*>* sim_objects,
                              uint64_t start, uint64_t end);

  friend class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_VTK_SIM_OBJECTS_H_
