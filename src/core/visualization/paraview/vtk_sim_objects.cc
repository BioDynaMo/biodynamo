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

#include "core/visualization/paraview/vtk_sim_objects.h"
// std
#include <algorithm>
#include <set>
#include <vector>
// ParaView
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
// ROOT
#include <TClass.h>
#include <TClassTable.h>
#include <TDataMember.h>
// BioDynaMo
#include "core/param/param.h"
#include "core/shape.h"
#include "core/sim_object/sim_object.h"
#include "core/simulation.h"
#include "core/util/jit.h"
#include "core/visualization/paraview/jit_helper.h"
#include "core/visualization/paraview/parallel_vtu_writer.h"

#include "core/sim_object/cell.h"

namespace bdm {

// FIXME
// struct PopulateDataArraysFunctorImpl1 : public PopulateDataArraysFunctor {
//   PopulateDataArraysFunctorImpl1(VtkSimObjects* so_grid, int tid)
//           : PopulateDataArraysFunctor(so_grid, tid) {}
//   void operator()(SimObject* so, SoHandle soh) {
//     auto idx = soh.GetElementIdx();
// PopulateDataArraysFunctor::SetTuple<bdm::Cell, double>(so, idx, so_grid_->array_indices_[0], 152);
// PopulateDataArraysFunctor::SetTuple<bdm::Cell, bdm::MathArray<double,3>>(so, idx, so_grid_->array_indices_[1], 104);
//   }
// };

struct CreateVtkDataArray1 : public Functor<void, VtkSimObjects*, int> {
  void operator()(VtkSimObjects* so_grid, int tid) {
    CreateVtkDataArray<bdm::Cell, double> f; f(tid, "diameter_", 152, so_grid); 
    CreateVtkDataArray<bdm::Cell, bdm::MathArray<double,3>> f1; f1(tid, "position_", 104, so_grid); 
  }
};

// -----------------------------------------------------------------------------
VtkSimObjects::VtkSimObjects(const char* type_name,
                     vtkCPDataDescription* data_description) {
  auto* param = Simulation::GetActive()->GetParam();
  auto* tinfo = ThreadInfo::GetInstance();
  if (param->export_visualization_) {
    data_.resize(tinfo->GetMaxThreads());
  } else {
    data_.resize(1);
  }
  #pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < data_.size(); ++i) {
    data_[i] = vtkUnstructuredGrid::New();
  }
  name_ = type_name;

  if (!param->export_visualization_) {
    data_description->AddInput(type_name);
    data_description->GetInputDescriptionByName(type_name)->SetGrid(data_[0]);
  }

  tclass_ = FindTClass();
  auto* tmp_instance = static_cast<SimObject*>(tclass_->New());
  shape_ = tmp_instance->GetShape();
  std::vector<std::string> data_members;
  InitializeDataMembers(tmp_instance, &data_members);

  // InitializeVtkSoGrid(this); FIXME
  // JitForEachDataMemberFunctor jitcreate(
  //     tclass_, data_members, "CreateVtkDataArraysFunctor",
  //     [](const std::string& functor_name, const std::vector<TDataMember*>& tdata_members) {
  //       std::stringstream sstr;
  //       sstr << "namespace bdm {\n\n"
  //            << "struct " << functor_name << " : public Functor<void, VtkSimObjects*, int> {\n"
  //            << "  void operator()(VtkSimObjects* so_grid, int tid) {\n";
  // 
  //       uint64_t counter = 0;
  //       for (auto* tdm : tdata_members) {
  //         // example:
  //         // CreateVtkDataArray<Cell, Double3>("position_", 123, so_grid);
  //         sstr << "CreateVtkDataArray<" 
  //              << tdm->GetClass()->GetName() << ", " 
  //              << tdm->GetTypeName() << ">("
  //              << "tid, \"" << tdm->GetName() << "\", " 
  //              << tdm->GetOffset() << ", so_grid);\n";
  //       }
  // 
  //       sstr << "  }\n";
  //       sstr << "};\n\n";
  //       sstr << "}  // namespace bdm\n";
  // 
  //       return sstr.str();
  // 
  //     });
  // jitcreate.Compile();
  // auto* create_functor = jitcreate.New<Functor<void, VtkSimObjects*, int>>(); FIXME
  auto create_functor = new CreateVtkDataArray1();
  for (uint64_t i = 0; i < data_.size(); ++i) {
    (*create_functor)(this, i);
  }
  delete create_functor;

  // FIXME
//   JitForEachDataMemberFunctor jitpopulate(
//       tclass_, data_members, "PopulateDataArraysFunctorImpl",
//       [](const std::string& functor_name, const std::vector<TDataMember*>& tdata_members) {
//         std::stringstream sstr;
//         sstr << "namespace bdm {\n\n"
//              << "struct " << functor_name << " : public PopulateDataArraysFunctor {\n"
//              << "  " << functor_name << "(VtkSimObjects* so_grid, int tid) \n"
//              << "          : PopulateDataArraysFunctor(so_grid, tid) {}\n"
//              << "  void operator()(SimObject* so, SoHandle soh) {\n"
//              << "    auto idx = soh.GetElementIdx();\n";
// 
//         uint64_t counter = 0;
//         for (auto* tdm : tdata_members) {
//           // example:
//           // PopulateDataArraysFunctor::SetTuple<Cell, Double3>(so, idx,
//           //     so_grid_->array_indices_[0], 104);
//           sstr << "PopulateDataArraysFunctor::SetTuple<"
//                << tdm->GetClass()->GetName() << ", " << tdm->GetTypeName()
//                << ">(so, idx, so_grid_->array_indices_[" << counter++ << "], "
//                << tdm->GetOffset() << ");\n";
//         }
// 
//         sstr << "  }\n";
//         sstr << "};\n\n";
//         sstr << "}  // namespace bdm\n";
//         return sstr.str();
//       });
//   jitpopulate.Compile();
//   // FIXME
//   //populate_arrays_ = jitpopulate.New<PopulateDataArraysFunctor>(
//       //Concat("reinterpret_cast<bdm::VtkSimObjects*>(", this, ")"));
//   populate_arrays_.resize(data_.size());
// #pragma omp parallel for schedule(static, 1)
//   for (uint64_t i = 0; i < populate_arrays_.size(); ++i) {
//     populate_arrays_[i] = new PopulateDataArraysFunctorImpl1(this, i);
//   }
}

// -----------------------------------------------------------------------------
VtkSimObjects::~VtkSimObjects() {
  name_ = "";
  for (auto& el : data_) {
    el->Delete();
  }
  data_.clear();
}

// -----------------------------------------------------------------------------
vtkUnstructuredGrid* VtkSimObjects::GetData(uint64_t idx) { return data_[idx]; }

// -----------------------------------------------------------------------------
Shape VtkSimObjects::GetShape() const { return shape_; }

// -----------------------------------------------------------------------------
TClass* VtkSimObjects::GetTClass() { return tclass_; }

// -----------------------------------------------------------------------------
void VtkSimObjects::Update(const std::vector<SimObject*>* sim_objects) {
  auto* param = Simulation::GetActive()->GetParam();
  if (param->export_visualization_) {
  #pragma omp parallel
    {
      auto* tinfo = ThreadInfo::GetInstance();
      auto tid = tinfo->GetMyThreadId();
      auto max_threads = tinfo->GetMaxThreads();

      // use static scheduling for now
      auto correction = sim_objects->size() % max_threads == 0 ? 0 : 1;
      auto chunk = sim_objects->size() / max_threads + correction;
      auto start = tid * chunk;
      auto end = std::min(sim_objects->size(), start + chunk);

      UpdateMappedDataArrays(tid, sim_objects, start, end);
    }
  } else {
    UpdateMappedDataArrays(0, sim_objects, 0, sim_objects->size());
  }

}

// -----------------------------------------------------------------------------
void VtkSimObjects::WriteToFile(uint64_t step) const {
  auto* sim = Simulation::GetActive();
  auto filename_prefix = Concat(name_, "-", step);
  
  ParallelVtuWriter writer;
  writer(sim->GetOutputDir(), filename_prefix, data_);
}

// -----------------------------------------------------------------------------
void VtkSimObjects::UpdateMappedDataArrays(uint64_t tid, const std::vector<SimObject*>* sim_objects, uint64_t start, uint64_t end) { 
  auto* parray = dynamic_cast<MappedDataArrayInterface*>(data_[tid]->GetPoints()->GetData());
  parray->Update(sim_objects, start, end);
  auto* point_data = data_[tid]->GetPointData();
  for (int i = 0; i < point_data->GetNumberOfArrays(); i++) {
    auto* array = dynamic_cast<MappedDataArrayInterface*>(point_data->GetArray(i));
    array->Update(sim_objects, start, end);
  }
}

// -----------------------------------------------------------------------------
TClass* VtkSimObjects::FindTClass() {
  const auto& tclass_vector = FindClassSlow(name_);
  if (tclass_vector.size() == 0) {
    // TODO message
    Log::Fatal("VtkSimObjects::VtkSimObjects", "Class not found");
  } else if (tclass_vector.size() == 0) {
    // TODO message
    Log::Fatal("VtkSimObjects::VtkSimObjects",
               "Class name ambigous. Add namespace modifier");
  }
  return tclass_vector[0];
}

// -----------------------------------------------------------------------------
void VtkSimObjects::InitializeDataMembers(SimObject* so,
                                      std::vector<std::string>* data_members) {
  std::set<std::string> dm_set = so->GetRequiredVisDataMembers();
  auto* param = Simulation::GetActive()->GetParam();
  for (auto& dm : param->visualize_sim_objects_.at(name_)) {
    dm_set.insert(dm);
  }
  data_members->resize(dm_set.size());
  std::copy(dm_set.begin(), dm_set.end(), data_members->begin());
}

}  // namespace bdm
