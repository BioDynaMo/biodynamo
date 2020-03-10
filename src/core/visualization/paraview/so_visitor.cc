// //
// -----------------------------------------------------------------------------
// //
// // Copyright (C) The BioDynaMo Project.
// // All Rights Reserved.
// //
// // Licensed under the Apache License, Version 2.0 (the "License");
// // you may not use this file except in compliance with the License.
// //
// // See the LICENSE file distributed with this work for details.
// // See the NOTICE file distributed with this work for additional information
// // regarding copyright ownership.
// //
// //
// -----------------------------------------------------------------------------
//
// #ifndef __ROOTCLING__
//
// #include <string>
//
// #include "core/visualization/paraview/helper.h"
// #include "core/visualization/paraview/so_visitor.h"
//
// #include <vtkDoubleArray.h>
// #include <vtkIntArray.h>
// #include <vtkNew.h>
// #include <vtkPoints.h>
//
// namespace bdm {
//
// struct ParaviewSoVisitor::ParaviewImpl {
//   VtkSoGrid* so_grid_;
// };
//
// template <typename TDataArray>
// TDataArray* GetDataArray(const std::string& dm_name, VtkSoGrid* so_grid,
//                          int components = 1) {
//   TDataArray* vtk_array = nullptr;
//   auto& data_arrays = so_grid->data_arrays_;
//   auto search = data_arrays.find(dm_name);
//   if (search != data_arrays.end()) {
//     auto& da_wrapper = search->second;
//     vtk_array = static_cast<TDataArray*>(da_wrapper.data_);
//
//     // reset
//     auto* scheduler = Simulation::GetActive()->GetScheduler();
//     if (da_wrapper.time_step_ != scheduler->GetSimulatedSteps()) {
//       vtk_array->Reset();
//       da_wrapper.time_step_ = scheduler->GetSimulatedSteps();
//     }
//
//   } else {
//     // create
//     vtkNew<TDataArray> new_vtk_array;
//     new_vtk_array->SetName(dm_name.c_str());
//     vtk_array = new_vtk_array.GetPointer();
//     vtk_array->SetNumberOfComponents(components);
//     auto* point_data = so_grid->data_->GetPointData();
//     point_data->AddArray(vtk_array);
//     data_arrays.insert({dm_name, VtkDataArrayWrapper(vtk_array)});
//   }
//
//   return vtk_array;
// }
//
// vtkDoubleArray* GetDouble3Array(const std::string& dm_name,
//                                 VtkSoGrid* so_grid) {
//   vtkDoubleArray* vtk_array = nullptr;
//   auto& data_arrays = so_grid->data_arrays_;
//   auto search = data_arrays.find(dm_name);
//   if (search != data_arrays.end()) {
//     auto& da_wrapper = search->second;
//     vtk_array = static_cast<vtkDoubleArray*>(da_wrapper.data_);
//
//     // reset
//     auto* scheduler = Simulation::GetActive()->GetScheduler();
//     if (da_wrapper.time_step_ != scheduler->GetSimulatedSteps()) {
//       vtk_array->Reset();
//       da_wrapper.time_step_ = scheduler->GetSimulatedSteps();
//     }
//
//   } else {
//     // create
//     vtkNew<vtkDoubleArray> new_vtk_array;
//     new_vtk_array->SetName(dm_name.c_str());
//     vtk_array = new_vtk_array.GetPointer();
//     vtk_array->SetNumberOfComponents(3);
//     data_arrays.insert({dm_name, VtkDataArrayWrapper(vtk_array)});
//     if (dm_name == "position_") {  // TODO(lukas) performance
//       vtkNew<vtkPoints> points;
//       points->SetData(vtk_array);
//       so_grid->data_->SetPoints(points.GetPointer());
//     } else if (dm_name == "mass_location_") {
//       // create points with position {0, 0, 0}
//       // BDMGlyph will rotate and translate based on the attribute data
//       vtkNew<vtkPoints> points;
//       points->SetData(vtk_array);
//       so_grid->data_->SetPoints(points.GetPointer());
//       so_grid->data_->GetPointData()->AddArray(vtk_array);
//     } else {
//       so_grid->data_->GetPointData()->AddArray(vtk_array);
//     }
//   }
//
//   return vtk_array;
// }
//
// ParaviewSoVisitor::ParaviewSoVisitor(VtkSoGrid* so_grid) {
//   impl_ = std::unique_ptr<ParaviewSoVisitor::ParaviewImpl>(
//       new ParaviewSoVisitor::ParaviewImpl());
//   impl_->so_grid_ = so_grid;
// }
// ParaviewSoVisitor::~ParaviewSoVisitor() {}
//
// void ParaviewSoVisitor::Visit(const std::string& dm_name, size_t
// type_hash_code,
//                               const void* data) {
//   if (type_hash_code == typeid(double).hash_code()) {
//     Double(dm_name, data);
//   } else if (type_hash_code == typeid(int).hash_code()) {
//     Int(dm_name, data);
//   } else if (type_hash_code == typeid(uint64_t).hash_code()) {
//     Uint64T(dm_name, data);
//   } else if (type_hash_code == typeid(Double3).hash_code()) {
//     MathArray3(dm_name, data);
//   } else if (type_hash_code == typeid(std::array<int, 3>).hash_code()) {
//     Int3(dm_name, data);
//   } else {
//     Log::Fatal("ParaviewSoVisitor::Visit",
//                "This data member is not supported for visualization");
//   }
// }
//
// void ParaviewSoVisitor::Double(const std::string& dm_name, const void* d) {
//   auto& data = *reinterpret_cast<const double*>(d);
//   auto* vtk_array = GetDataArray<vtkDoubleArray>(dm_name, impl_->so_grid_);
//   vtk_array->InsertNextTuple1(data);
// }
//
// void ParaviewSoVisitor::MathArray3(const std::string& dm_name, const void* d)
// {
//   auto& data = *reinterpret_cast<const Double3*>(d);
//   auto* vtk_array = GetDouble3Array(dm_name, impl_->so_grid_);
//   // TODO(lukas, ahmad) is there a better way?
//   vtk_array->InsertNextTuple3(data[0], data[1], data[2]);
// }
//
// void ParaviewSoVisitor::Int(const std::string& dm_name, const void* d) {
//   auto& data = *reinterpret_cast<const int*>(d);
//   auto* vtk_array = GetDataArray<vtkIntArray>(dm_name, impl_->so_grid_);
//   vtk_array->InsertNextTuple1(data);
// }
//
// void ParaviewSoVisitor::Uint64T(const std::string& dm_name, const void* d) {
//   auto& data = *reinterpret_cast<const uint64_t*>(d);
//   auto* vtk_array = GetDataArray<vtkIntArray>(dm_name, impl_->so_grid_);
//   vtk_array->InsertNextTuple1(data);
// }
//
// void ParaviewSoVisitor::Int3(const std::string& dm_name, const void* d) {
//   auto& data = *reinterpret_cast<const std::array<int, 3>*>(d);
//   auto* vtk_array = GetDataArray<vtkIntArray>(dm_name, impl_->so_grid_, 3);
//   // TODO(lukas, ahmad) is there a better way?
//   vtk_array->InsertNextTuple3(data[0], data[1], data[2]);
// }
//
// }  // namespace bdm
//
// #endif  // ifndef __ROOTCLING__
