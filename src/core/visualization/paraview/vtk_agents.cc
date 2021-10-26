// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
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

#include "core/visualization/paraview/vtk_agents.h"
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
#include "core/agent/agent.h"
#include "core/param/param.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/util/jit.h"
#include "core/visualization/paraview/jit_helper.h"
#include "core/visualization/paraview/parallel_vtu_writer.h"

#include "core/agent/cell.h"

namespace bdm {

// -----------------------------------------------------------------------------
VtkAgents::VtkAgents(const char* type_name,
                     vtkCPDataDescription* data_description) {
  auto* param = Simulation::GetActive()->GetParam();
  auto* tinfo = ThreadInfo::GetInstance();
  if (param->export_visualization) {
    data_.resize(tinfo->GetMaxThreads());
  } else {
    data_.resize(1);
  }
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < data_.size(); ++i) {
    data_[i] = vtkUnstructuredGrid::New();
  }
  name_ = type_name;

  if (!param->export_visualization) {
    data_description->AddInput(type_name);
    data_description->GetInputDescriptionByName(type_name)->SetGrid(data_[0]);
  }

  tclass_ = FindTClass();
  auto* tmp_instance = static_cast<Agent*>(tclass_->New());
  shape_ = tmp_instance->GetShape();
  std::vector<std::string> data_members;
  InitializeDataMembers(tmp_instance, &data_members);

  JitForEachDataMemberFunctor jitcreate(
      tclass_, data_members, "CreateVtkDataArrays",
      [](const std::string& functor_name,
         const std::vector<TDataMember*>& tdata_members) {
        std::stringstream sstr;
        sstr << "namespace bdm {\n\n"
             << "struct R__CLING_PTRCHECK(off) " << functor_name
             << " : public Functor<void, VtkAgents*, int> {\n"
             << "  void operator()(VtkAgents* agent_grid, int tid) {\n";

        for (auto* tdm : tdata_members) {
          // example:
          // { CreateVtkDataArray<Cell, Double3> f; f(tid, "position_", 123,
          // agent_grid); }
          sstr << "{ CreateVtkDataArray<" << tdm->GetClass()->GetName() << ", "
               << tdm->GetTypeName() << ">f; f("
               << "tid, \"" << tdm->GetName() << "\", " << tdm->GetOffset()
               << ", agent_grid); }\n";
        }

        sstr << "  }\n";
        sstr << "};\n\n";
        sstr << "}  // namespace bdm\n";

        return sstr.str();
      });
  jitcreate.Compile();
  auto* create_functor = jitcreate.New<Functor<void, VtkAgents*, int>>();
  for (uint64_t i = 0; i < data_.size(); ++i) {
    (*create_functor)(this, i);
  }
  delete create_functor;
}

// -----------------------------------------------------------------------------
VtkAgents::~VtkAgents() {
  name_ = "";
  for (auto& el : data_) {
    el->Delete();
  }
  data_.clear();
}

// -----------------------------------------------------------------------------
vtkUnstructuredGrid* VtkAgents::GetData(uint64_t idx) { return data_[idx]; }

// -----------------------------------------------------------------------------
Shape VtkAgents::GetShape() const { return shape_; }

// -----------------------------------------------------------------------------
TClass* VtkAgents::GetTClass() { return tclass_; }

// -----------------------------------------------------------------------------
void VtkAgents::Update(const std::vector<Agent*>* agents) {
  auto* param = Simulation::GetActive()->GetParam();
  if (param->export_visualization) {
#pragma omp parallel
    {
      auto* tinfo = ThreadInfo::GetInstance();
      auto tid = tinfo->GetMyThreadId();
      auto max_threads = tinfo->GetMaxThreads();

      // use static scheduling for now
      auto correction = agents->size() % max_threads == 0 ? 0 : 1;
      auto chunk = agents->size() / max_threads + correction;
      auto start = tid * chunk;
      auto end = std::min(agents->size(), start + chunk);

      UpdateMappedDataArrays(tid, agents, start, end);
    }
  } else {
    UpdateMappedDataArrays(0, agents, 0, agents->size());
  }
}

// -----------------------------------------------------------------------------
void VtkAgents::WriteToFile(uint64_t step) const {
  auto* sim = Simulation::GetActive();
  auto filename_prefix = Concat(name_, "-", step);

  ParallelVtuWriter writer;
  writer(sim->GetOutputDir(), filename_prefix, data_);
}

// -----------------------------------------------------------------------------
void VtkAgents::UpdateMappedDataArrays(uint64_t tid,
                                       const std::vector<Agent*>* agents,
                                       uint64_t start, uint64_t end) {
  auto* parray = dynamic_cast<MappedDataArrayInterface*>(
      data_[tid]->GetPoints()->GetData());
  parray->Update(agents, start, end);
  auto* point_data = data_[tid]->GetPointData();
  for (int i = 0; i < point_data->GetNumberOfArrays(); i++) {
    auto* array =
        dynamic_cast<MappedDataArrayInterface*>(point_data->GetArray(i));
    array->Update(agents, start, end);
  }
}

// -----------------------------------------------------------------------------
TClass* VtkAgents::FindTClass() {
  const auto& tclass_vector = FindClassSlow(name_);
  if (tclass_vector.size() == 0) {
    Log::Fatal("VtkAgents::VtkAgents",
               Concat("Could not find class: ", name_).c_str());
  } else if (tclass_vector.size() == 0) {
    std::stringstream str;
    for (auto& tc : tclass_vector) {
      str << tc->GetName() << std::endl;
    }
    Log::Fatal("VtkAgents::VtkAgents",
               Concat("Found multiple classes with name : ", name_,
                      ". See list below. Fix this issue by adding a namespace "
                      "modifier.\n'",
                      str.str())
                   .c_str());
  }
  return tclass_vector[0];
}

// -----------------------------------------------------------------------------
void VtkAgents::InitializeDataMembers(Agent* agent,
                                      std::vector<std::string>* data_members) {
  std::set<std::string> dm_set = agent->GetRequiredVisDataMembers();
  auto* param = Simulation::GetActive()->GetParam();
  for (auto& dm : param->visualize_agents.at(name_)) {
    dm_set.insert(dm);
  }
  data_members->resize(dm_set.size());
  std::copy(dm_set.begin(), dm_set.end(), data_members->begin());
}

}  // namespace bdm
