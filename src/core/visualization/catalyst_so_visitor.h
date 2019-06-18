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

#ifndef CORE_VISUALIZATION_CATALYST_SO_VISITOR_H_
#define CORE_VISUALIZATION_CATALYST_SO_VISITOR_H_

// check for ROOTCLING was necessary, due to ambigous reference to namespace
// detail when using ROOT I/O
#if defined(USE_CATALYST) && !defined(__ROOTCLING__)

#include <string>

#include "core/container/math_array.h"
#include "core/scheduler.h"
#include "core/sim_object/so_visitor.h"
#include "core/simulation.h"
#include "core/visualization/catalyst_helper_structs.h"

namespace bdm {

/// This simulation object visitor is used to extract data from simulation
/// objects. It also creates the required vtk data structures and resets them
/// at the beginning of each iteration.
class CatalystSoVisitor : public SoVisitor {
 public:
  explicit CatalystSoVisitor(VtkSoGrid* so_grid) : so_grid_(so_grid) {}
  virtual ~CatalystSoVisitor() {}

  void Visit(const std::string& dm_name, size_t type_hash_code,
             const void* data) override {
    if (type_hash_code == typeid(double).hash_code()) {
      Double(dm_name, data);
    } else if (type_hash_code == typeid(int).hash_code()) {
      Int(dm_name, data);
    } else if (type_hash_code == typeid(uint64_t).hash_code()) {
      Uint64T(dm_name, data);
    } else if (type_hash_code == typeid(Double3).hash_code()) {
      MathArray3(dm_name, data);
    } else if (type_hash_code == typeid(std::array<int, 3>).hash_code()) {
      Int3(dm_name, data);
    } else {
      Log::Fatal("CatalystSoVisitor::Visit",
                 "This data member is not supported for visualization");
    }
  }

  void Double(const std::string& dm_name, const void* d) {
    auto& data = *reinterpret_cast<const double*>(d);
    auto* vtk_array = GetDataArray<vtkDoubleArray>(dm_name);
    vtk_array->InsertNextTuple1(data);
  }

  void MathArray3(const std::string& dm_name, const void* d) {
    auto& data = *reinterpret_cast<const Double3*>(d);
    auto* vtk_array = GetDouble3Array(dm_name);
    // TODO(lukas, ahmad) is there a better way?
    vtk_array->InsertNextTuple3(data[0], data[1], data[2]);
  }

  void Int(const std::string& dm_name, const void* d) {
    auto& data = *reinterpret_cast<const int*>(d);
    auto* vtk_array = GetDataArray<vtkIntArray>(dm_name);
    vtk_array->InsertNextTuple1(data);
  }

  void Uint64T(const std::string& dm_name, const void* d) {
    auto& data = *reinterpret_cast<const uint64_t*>(d);
    auto* vtk_array = GetDataArray<vtkIntArray>(dm_name);
    vtk_array->InsertNextTuple1(data);
  }

  void Int3(const std::string& dm_name, const void* d) {
    auto& data = *reinterpret_cast<const std::array<int, 3>*>(d);
    auto* vtk_array = GetDataArray<vtkIntArray>(dm_name, 3);
    // TODO(lukas, ahmad) is there a better way?
    vtk_array->InsertNextTuple3(data[0], data[1], data[2]);
  }

 private:
  VtkSoGrid* so_grid_;

  template <typename TDataArray>
  TDataArray* GetDataArray(const std::string& dm_name,
                           int components = 1) const {
    TDataArray* vtk_array = nullptr;
    auto& data_arrays = so_grid_->data_arrays_;
    auto search = data_arrays.find(dm_name);
    if (search != data_arrays.end()) {
      auto& da_wrapper = search->second;
      vtk_array = static_cast<TDataArray*>(da_wrapper.data_);

      // reset
      auto* scheduler = Simulation::GetActive()->GetScheduler();
      if (da_wrapper.time_step_ != scheduler->GetSimulatedSteps()) {
        vtk_array->Reset();
        da_wrapper.time_step_ = scheduler->GetSimulatedSteps();
      }

    } else {
      // create
      vtkNew<TDataArray> new_vtk_array;
      new_vtk_array->SetName(dm_name.c_str());
      vtk_array = new_vtk_array.GetPointer();
      vtk_array->SetNumberOfComponents(components);
      auto* point_data = so_grid_->data_->GetPointData();
      point_data->AddArray(vtk_array);
      data_arrays.insert({dm_name, VtkDataArrayWrapper(vtk_array)});
    }

    return vtk_array;
  }

  vtkDoubleArray* GetDouble3Array(const std::string& dm_name) const {
    vtkDoubleArray* vtk_array = nullptr;
    auto& data_arrays = so_grid_->data_arrays_;
    auto search = data_arrays.find(dm_name);
    if (search != data_arrays.end()) {
      auto& da_wrapper = search->second;
      vtk_array = static_cast<vtkDoubleArray*>(da_wrapper.data_);

      // reset
      auto* scheduler = Simulation::GetActive()->GetScheduler();
      if (da_wrapper.time_step_ != scheduler->GetSimulatedSteps()) {
        vtk_array->Reset();
        da_wrapper.time_step_ = scheduler->GetSimulatedSteps();
      }

    } else {
      // create
      vtkNew<vtkDoubleArray> new_vtk_array;
      new_vtk_array->SetName(dm_name.c_str());
      vtk_array = new_vtk_array.GetPointer();
      vtk_array->SetNumberOfComponents(3);
      data_arrays.insert({dm_name, VtkDataArrayWrapper(vtk_array)});
      if (dm_name == "position_") {  // TODO(lukas) performance
        vtkNew<vtkPoints> points;
        points->SetData(vtk_array);
        so_grid_->data_->SetPoints(points.GetPointer());
      } else if (dm_name == "mass_location_") {
        // create points with position {0, 0, 0}
        // BDMGlyph will rotate and translate based on the attribute data
        vtkNew<vtkPoints> points;
        points->SetData(vtk_array);
        so_grid_->data_->SetPoints(points.GetPointer());
        so_grid_->data_->GetPointData()->AddArray(vtk_array);
      } else {
        so_grid_->data_->GetPointData()->AddArray(vtk_array);
      }
    }

    return vtk_array;
  }
};

}  // namespace bdm

#endif  // defined(USE_CATALYST) && !defined(__ROOTCLING__)

#endif  // CORE_VISUALIZATION_CATALYST_SO_VISITOR_H_
