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

#include "core/visualization/paraview/helper.h"
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

#include "core/sim_object/cell.h"

namespace bdm {

// FIXME
// struct PopulateDataArraysFunctorImpl1 : public PopulateDataArraysFunctor {
//   PopulateDataArraysFunctorImpl1(VtkSoGrid* so_grid, int tid)
//           : PopulateDataArraysFunctor(so_grid, tid) {}
//   void operator()(SimObject* so, SoHandle soh) {
//     auto idx = soh.GetElementIdx();
// PopulateDataArraysFunctor::SetTuple<bdm::Cell, double>(so, idx, so_grid_->array_indices_[0], 152);
// PopulateDataArraysFunctor::SetTuple<bdm::Cell, bdm::MathArray<double,3>>(so, idx, so_grid_->array_indices_[1], 104);
//   }
// };

struct CreateVtkDataArray1 : public Functor<void, VtkSoGrid*, int> {
  void operator()(VtkSoGrid* so_grid, int tid) {
    CreateVtkDataArray<bdm::Cell, double> f; f(tid, "diameter_", 152, so_grid); 
    CreateVtkDataArray<bdm::Cell, bdm::MathArray<double,3>> f1; f1(tid, "position_", 104, so_grid); 
  }
};

VtkSoGrid::VtkSoGrid(const char* type_name,
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

  tclass_ = GetTClass();
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
  //            << "struct " << functor_name << " : public Functor<void, VtkSoGrid*, int> {\n"
  //            << "  void operator()(VtkSoGrid* so_grid, int tid) {\n";
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
  // auto* create_functor = jitcreate.New<Functor<void, VtkSoGrid*, int>>(); FIXME
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
//              << "  " << functor_name << "(VtkSoGrid* so_grid, int tid) \n"
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
//       //Concat("reinterpret_cast<bdm::VtkSoGrid*>(", this, ")"));
//   populate_arrays_.resize(data_.size());
// #pragma omp parallel for schedule(static, 1)
//   for (uint64_t i = 0; i < populate_arrays_.size(); ++i) {
//     populate_arrays_[i] = new PopulateDataArraysFunctorImpl1(this, i);
//   }
}

VtkSoGrid::~VtkSoGrid() {
  name_ = "";
  for (auto& el : data_) {
    el->Delete();
  }
  data_.clear();
}

void VtkSoGrid::UpdateMappedDataArrays(uint64_t tid, const std::vector<SimObject*>* sim_objects, uint64_t start, uint64_t end) { 
  auto* parray = dynamic_cast<MappedDataArrayInterface*>(data_[tid]->GetPoints()->GetData());
  parray->Update(sim_objects, start, end);
  auto* point_data = data_[tid]->GetPointData();
  for (int i = 0; i < point_data->GetNumberOfArrays(); i++) {
    auto* array = dynamic_cast<MappedDataArrayInterface*>(point_data->GetArray(i));
    array->Update(sim_objects, start, end);
  }
}

TClass* VtkSoGrid::GetTClass() {
  const auto& tclass_vector = FindClassSlow(name_);
  if (tclass_vector.size() == 0) {
    // TODO message
    Log::Fatal("VtkSoGrid::VtkSoGrid", "Class not found");
  } else if (tclass_vector.size() == 0) {
    // TODO message
    Log::Fatal("VtkSoGrid::VtkSoGrid",
               "Class name ambigous. Add namespace modifier");
  }
  return tclass_vector[0];
}

void VtkSoGrid::InitializeDataMembers(SimObject* so,
                                      std::vector<std::string>* data_members) {
  std::set<std::string> dm_set = so->GetRequiredVisDataMembers();
  auto* param = Simulation::GetActive()->GetParam();
  for (auto& dm : param->visualize_sim_objects_.at(name_)) {
    dm_set.insert(dm);
  }
  data_members->resize(dm_set.size());
  std::copy(dm_set.begin(), dm_set.end(), data_members->begin());
}

// -----------------------------------------------------------------------------
std::string GenerateSimulationInfoJson(
    const std::unordered_map<std::string, VtkSoGrid*>& vtk_so_grids,
    const std::unordered_map<std::string, VtkDiffusionGrid*>& vtk_dgrids) {
  auto* sim = Simulation::GetActive();
  auto* param = sim->GetParam();
  // simulation objects
  std::stringstream sim_objects;
  uint64_t num_sim_objects = param->visualize_sim_objects_.size();
  uint64_t counter = 0;
  for (const auto& entry : param->visualize_sim_objects_) {
    std::string so_name = entry.first;

    auto search = vtk_so_grids.find(so_name);
    if (search == vtk_so_grids.end()) {
      continue;
    }
    auto* so_grid = search->second;

    sim_objects << "    {" << std::endl
                << "      \"name\":\"" << so_name << "\"," << std::endl;
    if (so_grid->shape_ == Shape::kSphere) {
      sim_objects << "      \"glyph\":\"Glyph\"," << std::endl
                  << "      \"shape\":\"Sphere\"," << std::endl
                  << "      \"scaling_attribute\":\"diameter_\"" << std::endl;
    } else if (so_grid->shape_ == kCylinder) {
      sim_objects << "      \"glyph\":\"BDMGlyph\"," << std::endl
                  << "      \"shape\":\"Cylinder\"," << std::endl
                  << "      \"x_scaling_attribute\":\"diameter_\"," << std::endl
                  << "      \"y_scaling_attribute\":\"actual_length_\","
                  << std::endl
                  << "      \"z_scaling_attribute\":\"diameter_\"," << std::endl
                  << "      \"Vectors\":\"spring_axis_\"," << std::endl
                  << "      \"MassLocation\":\"mass_location_\"" << std::endl;
    }
    sim_objects << "    }";
    if (counter != num_sim_objects - 1) {
      sim_objects << ",";
    }
    sim_objects << std::endl;
    counter++;
  }

  // extracellular substances
  std::stringstream substances;
  uint64_t num_substances = param->visualize_diffusion_.size();
  for (uint64_t i = 0; i < num_substances; i++) {
    auto& name = param->visualize_diffusion_[i].name_;

    auto search = vtk_dgrids.find(name);
    if (search == vtk_dgrids.end()) {
      continue;
    }
    auto* dgrid = search->second;
    // user wanted to export this substance, but it did not exist during
    // the entire simulation
    if (!dgrid->used_) {
      Log::Warning(
          "Visualize Diffusion Grids",
          "You are trying to visualize diffusion grid ", name,
          ", but it has not been created during the entire simulation. "
          "Please make sure the names in the "
          "configuration file match the ones in the simulation.");
      num_substances--;
      continue;
    }
    substances << "    { \"name\":\"" << name << "\", ";
    std::string has_gradient =
        param->visualize_diffusion_[i].gradient_ ? "true" : "false";
    substances << "\"has_gradient\":\"" << has_gradient << "\" }";

    if (i != num_substances - 1) {
      substances << "," << std::endl;
    }
  }

  // write to file
  std::stringstream str;
  str << "{" << std::endl
      << "  \"simulation\": {" << std::endl
      << "    \"name\":\"" << sim->GetUniqueName() << "\"," << std::endl
      << "    \"result_dir\":\"" << sim->GetOutputDir() << "\"" << std::endl
      << "  }," << std::endl
      << "  \"sim_objects\": [" << std::endl
      << sim_objects.str() << "  ]," << std::endl
      << "  \"extracellular_substances\": [" << std::endl
      << substances.str() << std::endl
      << "  ]" << std::endl
      << "}" << std::endl;
  return str.str();
}

}  // namespace bdm
