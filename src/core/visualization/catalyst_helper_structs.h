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

#ifndef CORE_VISUALIZATION_CATALYST_HELPER_STRUCTS_H_
#define CORE_VISUALIZATION_CATALYST_HELPER_STRUCTS_H_

// check for ROOTCLING was necessary, due to ambigous reference to namespace
// detail when using ROOT I/O
#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

#include <set>
#include <string>
#include <unordered_map>
#include "core/param/param.h"
#include "core/shape.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"

#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>

namespace bdm {

/// Add data member `time_step_` to a vtkDataArray.
/// It is required to determine when the data array should be reset. (At the
/// beginning of each iteration)
struct VtkDataArrayWrapper {
  explicit VtkDataArrayWrapper(vtkDataArray* data) : data_(data) {}
  vtkDataArray* data_;
  uint64_t time_step_ = 0;
};

/// Adds additional data members to the `vtkUnstructuredGrid` required by
/// `CatalystAdaptor` to visualize simulation objects.
struct VtkSoGrid {
  VtkSoGrid(const char* type_name,
            const vtkNew<vtkCPDataDescription>& data_description) {
    data_ = vtkUnstructuredGrid::New();
    name_ = type_name;
    data_description->AddInput(type_name);
    data_description->GetInputDescriptionByName(type_name)->SetGrid(data_);
  }

  ~VtkSoGrid() {
    name_ = "";
    data_->Delete();
    data_ = nullptr;
    vis_data_members_.clear();
    data_arrays_.clear();
  }

  void Init(const SimObject* so) {
    auto type_name = so->GetTypeName();
    vis_data_members_ = so->GetRequiredVisDataMembers();
    auto* param = Simulation::GetActive()->GetParam();
    for (auto& dm : param->visualize_sim_objects_.at(type_name)) {
      vis_data_members_.insert(dm);
    }
    shape_ = so->GetShape();
    initialized_ = true;
  }

  bool initialized_ = false;
  std::string name_;
  vtkUnstructuredGrid* data_ = nullptr;
  Shape shape_;

  std::set<std::string> vis_data_members_;
  std::unordered_map<std::string, VtkDataArrayWrapper> data_arrays_;
};

/// Adds additional data members to the `vtkImageData` required by
/// `CatalystAdaptor` to visualize diffusion grid.
struct VtkDiffusionGrid {
  VtkDiffusionGrid(const std::string& name,
                   const vtkNew<vtkCPDataDescription>& data_description) {
    data_ = vtkImageData::New();
    name_ = name;

    // get visualization config
    auto* param = Simulation::GetActive()->GetParam();
    const Param::VisualizeDiffusion* vd = nullptr;
    for (auto& entry : param->visualize_diffusion_) {
      if (entry.name_ == name) {
        vd = &entry;
        break;
      }
    }

    // Add attribute data
    if (vd->concentration_) {
      vtkNew<vtkDoubleArray> concentration;
      concentration->SetName("Substance Concentration");
      concentration_ = concentration.GetPointer();
      data_->GetPointData()->AddArray(concentration.GetPointer());
    }
    if (vd->gradient_) {
      vtkNew<vtkDoubleArray> gradient;
      gradient->SetName("Diffusion Gradient");
      gradient->SetNumberOfComponents(3);
      gradient_ = gradient.GetPointer();
      data_->GetPointData()->AddArray(gradient.GetPointer());
    }

    data_description->AddInput(name.c_str());
    data_description->GetInputDescriptionByName(name.c_str())->SetGrid(data_);
  }

  ~VtkDiffusionGrid() {
    name_ = "";
    data_->Delete();
    data_ = nullptr;
    concentration_ = nullptr;
    gradient_ = nullptr;
  }

  void Init() { used_ = true; }

  bool used_ = false;
  std::string name_;
  vtkImageData* data_ = nullptr;
  vtkDoubleArray* concentration_ = nullptr;
  vtkDoubleArray* gradient_ = nullptr;
};

}  // namespace bdm

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

#endif  // CORE_VISUALIZATION_CATALYST_HELPER_STRUCTS_H_
