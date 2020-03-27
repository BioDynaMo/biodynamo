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
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/visualization/paraview/helper.h"
#include "core/visualization/paraview/mapped_data_array.h"

#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>

namespace bdm {

template <typename T>
struct GetVtkArrayType {};

template <>
struct GetVtkArrayType<double> {
  using type = MappedDataArray<double>;
};

template <>
struct GetVtkArrayType<int> {
  using type = MappedDataArray<int>;
};

template <>
struct GetVtkArrayType<uint64_t> {
  using type = MappedDataArray<int>;
};

template <typename T, std::size_t N>
struct GetVtkArrayType<MathArray<T, N>> {
  using type = typename GetVtkArrayType<T>::type;
};

template <typename T>
struct GetNumberOfComponents {
  static const int value = 1;
};

template <typename T, std::size_t N>
struct GetNumberOfComponents<MathArray<T, N>> {
  static const int value = N;
};

template <typename T, std::size_t N>
struct GetNumberOfComponents<std::array<T, N>> {
  static const int value = N;
};

template <typename T>
struct IsArray : std::false_type {};

template <typename T, size_t N>
struct IsArray<std::array<T, N>> : std::true_type {};

template <typename T, std::size_t N>
struct IsArray<MathArray<T, N>> : std::true_type {};

template <typename TReturn, typename TClass, typename TDataMember>
struct GetDataMemberFunctor : public Functor<TReturn, SimObject*> {
  GetDataMemberFunctor(uint64_t dm_offset) : dm_offset_(dm_offset) {}
 
  TReturn operator()(SimObject* so) final {
    return OperatorImpl(so);
  }
  
  template <typename TTDataMember = TDataMember>
  typename std::enable_if<IsArray<TTDataMember>::value, TReturn>::type
  OperatorImpl(SimObject* so) {
    auto* casted_so = static_cast<TClass*>(so);
    auto* data = reinterpret_cast<TDataMember*>(
                     reinterpret_cast<char*>(casted_so) + dm_offset_)
                     ->data();
    return const_cast<TReturn>(data);
  }

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<!IsArray<TTDataMember>::value, TReturn>::type
  OperatorImpl(SimObject* so) {
    auto* casted_so = static_cast<TClass*>(so);
    return reinterpret_cast<TDataMember*>(
                     reinterpret_cast<char*>(casted_so) + dm_offset_);
  }

 private:
  uint64_t dm_offset_;
};

template <typename TClass, typename TDataMember>
struct CreateVtkDataArray {

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<!std::is_same<TTDataMember, Double3>::value>::type
  operator()(uint64_t tid, const std::string& dm_name, uint64_t dm_offset, VtkSoGrid* so_grid) {
    using VtkArrayType = typename GetVtkArrayType<TDataMember>::type;
    using VtkValueType = typename VtkArrayType::ValueType;
    unsigned components = GetNumberOfComponents<TDataMember>::value;
    auto* access_dm = new GetDataMemberFunctor<VtkValueType*, TClass, TDataMember>(dm_offset);
    vtkNew<VtkArrayType> new_vtk_array;
    new_vtk_array->Initialize(dm_name, components, access_dm);
    auto* vtk_array = new_vtk_array.GetPointer();
    auto* point_data = so_grid->data_[tid]->GetPointData();
    point_data->AddArray(vtk_array);
  }

  template <typename TTDataMember = TDataMember>
  typename std::enable_if<std::is_same<TTDataMember, Double3>::value>::type
  operator()(uint64_t tid, const std::string& dm_name, uint64_t dm_offset, VtkSoGrid* so_grid) {
    auto* access_dm = new GetDataMemberFunctor<double*, TClass, Double3>(dm_offset);
    vtkNew<MappedDataArray<double>> new_vtk_array;
    new_vtk_array->Initialize(dm_name, 3, access_dm);
    auto* vtk_array = new_vtk_array.GetPointer();
    if (dm_name == "position_") {
      vtkNew<vtkPoints> points;
      points->SetData(vtk_array);
      so_grid->data_[tid]->SetPoints(points.GetPointer());
    } else if (dm_name == "mass_location_") {
      // create points with position {0, 0, 0}
      // BDMGlyph will rotate and translate based on the attribute data
      vtkNew<vtkPoints> points;
      points->SetData(vtk_array);
      so_grid->data_[tid]->SetPoints(points.GetPointer());
      so_grid->data_[tid]->GetPointData()->AddArray(vtk_array);
    } else {
      so_grid->data_[tid]->GetPointData()->AddArray(vtk_array);
    }
  }
};

struct PopulateDataArraysFunctor : public Functor<void, SimObject*, SoHandle> {
  VtkSoGrid* so_grid_;
  vtkUnstructuredGrid* grid_;
  vtkPointData* point_data_;

  PopulateDataArraysFunctor(VtkSoGrid* so_grid, int tid)
      : so_grid_(so_grid),
        grid_(so_grid->data_[tid]),
        point_data_(so_grid->data_[tid]->GetPointData()) {}

  template <typename TClass, typename TDataMember>
  typename std::enable_if<IsArray<TDataMember>::value>::type SetTuple(
      SimObject* so, uint64_t so_idx, int array_idx, uint64_t dm_offset) {
    auto* casted_so = static_cast<TClass*>(so);
    if (array_idx == -1) {
      auto* data = reinterpret_cast<TDataMember*>(
                       reinterpret_cast<char*>(casted_so) + dm_offset)
                       ->data();
      grid_->GetPoints()->GetData()->SetTuple(so_idx, data);
    } else {
      auto* data = reinterpret_cast<TDataMember*>(
                       reinterpret_cast<char*>(casted_so) + dm_offset)
                       ->data();
      point_data_->GetArray(array_idx)->SetTuple(so_idx, data);
    }
  }

  template <typename TClass, typename TDataMember>
  typename std::enable_if<!IsArray<TDataMember>::value>::type SetTuple(
      SimObject* so, uint64_t so_idx, int array_idx, uint64_t dm_offset) {
    auto* casted_so = static_cast<TClass*>(so);
    if (array_idx != -1) {
      auto* data = reinterpret_cast<TDataMember*>(
          reinterpret_cast<char*>(casted_so) + dm_offset);
      point_data_->GetArray(array_idx)->SetTuple(so_idx, data);
    }
  }
};

// clang-format off
/// The following functor is only needed to fix a bug in ROOT Cling
/// Without it it is unable to compile
/// `PopulateDataArraysFunctor::SetTuple<SimObject, Double3>(...)` \n
/// It throws the following error:\n
///
///     input_line_15:9:38: error: no matching member function for call to 'SetTuple'
///               PopulateDataArraysFunctor::SetTuple<Cell, Double3>(so, idx, -1, 104);
///               ~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~
///     libVisualizationAdaptor_dict dictionary payload:244:3: note: candidate template
///           ignored: substitution failure [with TClass = bdm::Cell, TDataMember = bdm::MathArray<double, 3>]: a
///           lambda expression may not appear inside of a constant expression
// clang-format on
struct ROOTClingFix : public PopulateDataArraysFunctor {
  void operator()(SimObject* so, SoHandle soh) {
    PopulateDataArraysFunctor::SetTuple<SimObject, Double3>(so, 0, 0, 0);
  }
};

}  // namespace bdm

#endif  // CORE_VISUALIZATION_PARAVIEW_JIT_HELPER_H_
