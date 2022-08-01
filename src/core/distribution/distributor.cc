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

#include "core/distribution/distributor.h"

#include "core/distribution/distribution_param.h"
#include "core/environment/uniform_grid_environment.h"
#include "core/simulation.h"
#include "core/util/log.h"

#include <stk_io/FillMesh.hpp>         // stk::io::fill_mesh
#include <stk_mesh/base/BulkData.hpp>  // for BulkData
#include <stk_mesh/base/Field.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MetaData.hpp>  // for MetaData, put_field
#include <stk_mesh/baseImpl/Visitors.hpp>
#include <stk_util/parallel/ParallelReduce.hpp>

#include <stk_balance/balance.hpp>
#include <stk_balance/balanceUtils.hpp>
#include <stk_balance/internal/privateDeclarations.hpp>
#include <stk_util/environment/CPUTime.hpp>
#include <stk_util/environment/WallTime.hpp>

namespace bdm {
namespace experimental {

Distributor::Distributor() {}
Distributor::~Distributor() {}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
class SpatialSTKDistributor : public Distributor {
 private:
  static constexpr int kSpatialDimensions = 3;
  stk::mesh::MetaData meta_;
  stk::mesh::BulkData bulk_;
  stk::mesh::Field<int>* proc_owner_ = nullptr;
  stk::mesh::Field<float>* weights_ = nullptr;
  MathArray<int, 6> whole_space_;
  int32_t interaction_radius_;
  int32_t box_length_;

 public:
  SpatialSTKDistributor();
  virtual ~SpatialSTKDistributor();

 private:
  void InitializeMesh();
  void UpdateProcOwnerField();
};

// -----------------------------------------------------------------------------
SpatialSTKDistributor::SpatialSTKDistributor()
    : meta_(kSpatialDimensions, stk::mesh::entity_rank_names()),
      bulk_(meta_, MPI_COMM_WORLD, stk::mesh::BulkData::AUTO_AURA) {
  // sanity checks
  auto* sim = Simulation::GetActive();
  auto* space = sim->GetSimulationSpace();
  auto* env = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
  if (!env) {
    Log::Fatal("SpatialSTKDistributor",
               "The SpatialSTKDistributor currently requires the "
               "UniformGridEnvironment class. We detected a different "
               "Environment implementation.");
  } 
  interaction_radius_ = space->GetInteractionRadius();
  whole_space_ = space->GetWholeSpace();

  // setup fields
  float init_weight = 1.0;
  weights_ = &meta_.declare_field<stk::mesh::Field<float>>(
      stk::topology::ELEM_RANK, "Weights", 1);
  stk::mesh::put_field_on_mesh(*weights_, meta_.universal_part(), &init_weight);
  int init_proc = -1.0;
  proc_owner_ = &meta_.declare_field<stk::mesh::Field<int>>(
      stk::topology::ELEM_RANK, "ProcOwner", 1);
  stk::mesh::put_field_on_mesh(*proc_owner_, meta_.universal_part(),
                               &init_proc);

  InitializeMesh();
  UpdateProcOwnerField();
}

// -----------------------------------------------------------------------------
SpatialSTKDistributor::~SpatialSTKDistributor() {}

// -----------------------------------------------------------------------------
void SpatialSTKDistributor::InitializeMesh() {
  std::stringstream sstream;
  auto* param = Simulation::GetActive()->GetParam();

  auto* sim = Simulation::GetActive();
  auto* env = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
  auto* dparam = sim->GetParam()->Get<DistributionParam>();

  box_length_ = env->GetBoxLength() * dparam->box_length_factor;
  int maxx, maxy, maxz;
  maxx = maxy = maxz = 9;
  sstream << "generated:" << maxx << "x" << maxy << "x" << maxz;
  stk::io::fill_mesh(sstream.str(), bulk_);
}

// -----------------------------------------------------------------------------
void SpatialSTKDistributor::UpdateProcOwnerField() {
  stk::mesh::EntityVector elements;
  stk::mesh::get_entities(bulk_, stk::topology::ELEM_RANK,
                          meta_.locally_owned_part(), elements);

  for (const stk::mesh::Entity& element : elements) {
    auto* data = stk::mesh::field_data(*proc_owner_, element);
    *data = bulk_.parallel_rank();
  }
}

// -----------------------------------------------------------------------------
Distributor* CreateDistributor(DistributorType type) {
  switch (type) {
    case kSpatialSTK:
      return new SpatialSTKDistributor();
    default:
      Log::Fatal("CreateDistributor", "The given distribution type ", type,
                 " is not supported.");
      return nullptr;
  }
}

}  // namespace experimental
}  // namespace bdm
