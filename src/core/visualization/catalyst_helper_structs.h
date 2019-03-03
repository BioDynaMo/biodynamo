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

#include <string>
#include <set>
#include <unordered_map>

#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>

namespace bdm {

/// Add data member `time_step_` to a vtkDataArray.
/// It is required to determine when the data array should be reset. (At the
/// beginning of each iteration)
struct VtkDataArrayWrapper {
  VtkDataArrayWrapper(vtkDataArray* data) : data_(data) {}
  vtkDataArray* data_;
  uint64_t time_step_ = 0;
};

/// Adds additional data members to the `vtkUnstructuredGrid` required by
/// `CatalystAdaptor` to visualize simulation objects.
struct VtkSoGrid {
  void Reset() {
    name_ = "";
    data_ = nullptr;
    vis_data_members_.clear();
    data_arrays_.clear();
  }

  std::string name_;
  vtkUnstructuredGrid* data_ = nullptr;

  std::set<std::string> vis_data_members_;
  std::unordered_map<std::string, VtkDataArrayWrapper> data_arrays_;
};

/// Adds additional data members to the `vtkImageData` required by
/// `CatalystAdaptor` to visualize diffusion grid.
struct VtkDiffusionGrid {
  void Reset() {
    name_ = "";
    data_ = nullptr;
    concentration_ = nullptr;
    gradient_ = nullptr;
  }

  std::string name_;
  vtkImageData* data_ = nullptr;
  vtkDoubleArray* concentration_ = nullptr;
  vtkDoubleArray* gradient_ = nullptr;
};

}  // namespace bdm

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

#endif  // CORE_VISUALIZATION_CATALYST_HELPER_STRUCTS_H_
