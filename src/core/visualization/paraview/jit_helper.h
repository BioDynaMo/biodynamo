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

#ifndef CORE_VISUALIZATION_PARAVIEW_JIT_HELPER_H_
#define CORE_VISUALIZATION_PARAVIEW_JIT_HELPER_H_

#include "core/container/math_array.h"
#include "core/functor.h"
#include "core/param/param.h"
#include "core/resource_manager.h"
#include "core/sim_object/so_uid.h"
#include "core/sim_object/so_pointer.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/util/type.h"
#include "core/visualization/paraview/helper.h"
#include "core/visualization/paraview/mapped_data_array.h"

#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>

namespace bdm {

// -----------------------------------------------------------------------------
template <typename T>
struct GetVtkValueType {};

// -----------------------------------------------------------------------------
template <>
struct GetVtkValueType<double> {
  using type = double;
};

// -----------------------------------------------------------------------------
template <>
struct GetVtkValueType<int> {
  using type = int;
};

// -----------------------------------------------------------------------------
template <>
struct GetVtkValueType<uint64_t> {
  using type = uint64_t;
};

// -----------------------------------------------------------------------------
template <>
struct GetVtkValueType<SoUid> {
  using type = uint64_t;
};

// -----------------------------------------------------------------------------
template <typename T>
struct GetVtkValueType<SoPointer<T>> {
  using type = uint64_t;
};

// -----------------------------------------------------------------------------
template <typename T, std::size_t N>
struct GetVtkValueType<MathArray<T, N>> {
  using type = typename GetVtkValueType<T>::type;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
template <typename T>
struct GetNumberOfComponents {
  static const int value = 1;
};

// -----------------------------------------------------------------------------
template <typename T, std::size_t N>
struct GetNumberOfComponents<MathArray<T, N>> {
  static const int value = N;
};

// -----------------------------------------------------------------------------
template <typename T, std::size_t N>
struct GetNumberOfComponents<std::array<T, N>> {
  static const int value = N;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
template <typename TClass, typename TDataMember>
struct CreateVtkDataArray {

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<!std::is_same<TTDataMember, Double3>::value>::type
  operator()(uint64_t tid, const std::string& dm_name, uint64_t dm_offset, VtkSimObjects* vtk_sim_objects) {
    using VtkValueType = typename GetVtkValueType<TDataMember>::type;
    using VtkArrayType = MappedDataArray<VtkValueType, TClass, TDataMember>;
    unsigned components = GetNumberOfComponents<TDataMember>::value;
    vtkNew<VtkArrayType> new_vtk_array;
    auto mode = Simulation::GetActive()->GetParam()->mapped_data_array_mode_;
    new_vtk_array->Initialize(mode, dm_name, components, dm_offset);
    auto* vtk_array = new_vtk_array.GetPointer();
    auto* point_data = vtk_sim_objects->GetData(tid)->GetPointData();
    point_data->AddArray(vtk_array);
  }

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<std::is_same<TTDataMember, Double3>::value>::type
  operator()(uint64_t tid, const std::string& dm_name, uint64_t dm_offset, VtkSimObjects* vtk_sim_objects) {
    using VtkArrayType = MappedDataArray<double, TClass, TDataMember>;
    vtkNew<VtkArrayType> new_vtk_array;
    auto mode = Simulation::GetActive()->GetParam()->mapped_data_array_mode_;
    new_vtk_array->Initialize(mode, dm_name, 3, dm_offset);
    auto* vtk_array = new_vtk_array.GetPointer();
    if (dm_name == "position_") {
      vtkNew<vtkPoints> points;
      points->SetData(vtk_array);
      vtk_sim_objects->GetData(tid)->SetPoints(points.GetPointer());
    } else if (dm_name == "mass_location_") {
      // create points with position {0, 0, 0}
      // BDMGlyph will rotate and translate based on the attribute data
      vtkNew<vtkPoints> points;
      points->SetData(vtk_array);
      vtk_sim_objects->GetData(tid)->SetPoints(points.GetPointer());
      vtk_sim_objects->GetData(tid)->GetPointData()->AddArray(vtk_array);
    } else {
      vtk_sim_objects->GetData(tid)->GetPointData()->AddArray(vtk_array);
    }
  }
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_JIT_HELPER_H_
