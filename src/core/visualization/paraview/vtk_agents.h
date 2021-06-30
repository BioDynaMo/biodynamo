// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_VISUALIZATION_PARAVIEW_VTK_AGENTS_H_
#define CORE_VISUALIZATION_PARAVIEW_VTK_AGENTS_H_

// std
#include <string>
#include <vector>
// Paraview
#include <vtkCPDataDescription.h>
#include <vtkUnstructuredGrid.h>
// BioDynaMo
#include "core/agent/agent.h"
#include "core/shape.h"

class TClass;

namespace bdm {

class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;

/// Adds additional data members to the `vtkUnstructuredGrid` required by
/// `ParaviewAdaptor` to visualize agents.
class VtkAgents {
 public:
  VtkAgents(const char* type_name, vtkCPDataDescription* data_description);

  ~VtkAgents();

  vtkUnstructuredGrid* GetData(uint64_t idx);
  Shape GetShape() const;
  TClass* GetTClass();
  void Update(const std::vector<Agent*>* agents);
  void WriteToFile(uint64_t step) const;

 private:
  std::string name_;
  TClass* tclass_;
  std::vector<vtkUnstructuredGrid*> data_;
  Shape shape_;

  TClass* FindTClass();
  void InitializeDataMembers(Agent* agent,
                             std::vector<std::string>* data_members);
  void UpdateMappedDataArrays(uint64_t tid, const std::vector<Agent*>* agents,
                              uint64_t start, uint64_t end);

  friend class ParaviewAdaptorTest_GenerateSimulationInfoJson_Test;
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_VTK_AGENTS_H_
