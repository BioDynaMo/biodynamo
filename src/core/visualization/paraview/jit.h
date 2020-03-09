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

#ifndef CORE_VISUALIZATION_PARAVIEW_JIT_H_
#define CORE_VISUALIZATION_PARAVIEW_JIT_H_

#include "core/container/math_array.h"
#include "core/functor.h"
#include "core/visualization/paraview/helper.h"
#include "core/simulation.h"
#include "core/param/param.h"
#include "core/sim_object/sim_object.h"
#include "core/resource_manager.h"
#include "core/sim_object/cell.h" // FIXME remove

#ifndef __ROOTCLING__

#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>

namespace bdm {

template <typename T>
struct GetVtkArrayType {};

template <>
struct GetVtkArrayType<double> {
  using type = vtkDoubleArray;
};

template <>
struct GetVtkArrayType<int> {
  using type = vtkDoubleArray;
};

template <>
struct GetVtkArrayType<uint64_t> {
  using type = vtkDoubleArray;
};

template <typename T>
inline void CreateVtkDataArray(const std::string& dm_name, uint64_t components, VtkSoGrid* so_grid) {
  using VtkArrayType = typename GetVtkArrayType<T>::type;
  vtkNew<VtkArrayType> new_vtk_array;
  new_vtk_array->SetName(dm_name.c_str());
  auto* vtk_array = new_vtk_array.GetPointer();
  vtk_array->SetNumberOfComponents(components);
  auto* point_data = so_grid->data_->GetPointData();
  point_data->AddArray(vtk_array);
}

template <>
inline void CreateVtkDataArray<Double3>(const std::string& dm_name, uint64_t components, VtkSoGrid* so_grid) {
  vtkNew<vtkDoubleArray> new_vtk_array;
  new_vtk_array->SetName(dm_name.c_str());
  auto* vtk_array = new_vtk_array.GetPointer();
  vtk_array->SetNumberOfComponents(3);
  if (dm_name == "position_") {
    vtkNew<vtkPoints> points;
    points->SetData(vtk_array);
    so_grid->data_->SetPoints(points.GetPointer());
  } else if (dm_name == "mass_location_") {
    // create points with position {0, 0, 0}
    // BDMGlyph will rotate and translate based on the attribute data
    vtkNew<vtkPoints> points;
    points->SetData(vtk_array);
    so_grid->data_->SetPoints(points.GetPointer());
    so_grid->data_->GetPointData()->AddArray(vtk_array);
  } else {
    so_grid->data_->GetPointData()->AddArray(vtk_array);
  }
}

inline void InitializeVtkSoGrid(VtkSoGrid* so_grid) {
  so_grid->shape_ = Shape::kSphere;
  so_grid->vis_data_members_ = {"position_", "diameter_"};// FIXME so->GetRequiredVisDataMembers();
  auto* param = Simulation::GetActive()->GetParam();
  for (auto& dm : param->visualize_sim_objects_.at(so_grid->name_)) {
    so_grid->vis_data_members_.insert(dm);
  }
  CreateVtkDataArray<Double3>("position_", 3, so_grid);
  CreateVtkDataArray<double>("diameter_", 1, so_grid);
}

struct PopulateDataArraysFunctor : public Functor<void, SimObject*, SoHandle> {
  vtkUnstructuredGrid* grid_;
  vtkPointData* point_data_;

  PopulateDataArraysFunctor(VtkSoGrid* so_grid) : grid_(so_grid->data_), point_data_(so_grid->data_->GetPointData()) {}

  void operator()(SimObject* so, SoHandle soh) {
    auto idx = soh.GetElementIdx();
    auto* cell = static_cast<Cell*>(so);
    const auto& pos = cell->GetPosition();
    auto* array = grid_->GetPoints()->GetData();
    // array->SetTuple(idx, pos.data());
    array->InsertNextTuple3(pos[0], pos[1], pos[2]);
    static_cast<vtkDoubleArray*>(point_data_->GetArray("diameter_"))->InsertNextTuple1(cell->GetDiameter());
  }
};


}  // namespace bdm

#endif  // __ROOTCLING__
#endif  // CORE_VISUALIZATION_PARAVIEW_JIT_H_
