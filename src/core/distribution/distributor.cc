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

#ifdef USE_DSE

#include <set>

#include "core/distribution/distribution_param.h"
#include "core/distribution/communication.h"
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
  int32_t interaction_radius_;
  int32_t box_length_;
  /// copy from SimulationSpace to determine if it was changed during the simulation.
  SimulationSpace::Space whole_space_;
  /// Number of boxes in each dimension for the whole simulation space
  MathArray<int, 3> num_boxes_axis_;
  /// Number of boxes in the xy plane for the whole simulation space
  uint64_t num_boxes_xy_;
  /// Contains all MPI ranks of neighboring processors
  /// i.e. processors that share an aura 
  std::set<int> neighbor_ranks_;

 public:
  SpatialSTKDistributor();
  virtual ~SpatialSTKDistributor();

  void MigrateAgents() override;

 private:
  void InitializeMesh();
  void UpdateProcOwnerField();
  void UpdateLocalSpace();
  void UpdateNeighborRanks();
  SimulationSpace::Space BoxToSpaceCoordinates(
      const MathArray<int, 6>& box_space) const;
  MathArray<int, 3> GetBoxCoordinates(size_t id) const;
  MathArray<int, 3> GetBoxCoordinates(const Real3& pos) const;
  uint64_t GetId(const MathArray<int, 3>& box_coord);
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
  auto* sim = Simulation::GetActive();
  auto* env = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
  auto* space = dynamic_cast<DistributedSimSpace*>(sim->GetSimulationSpace());
  auto* dparam = sim->GetParam()->Get<DistributionParam>();

  if (!env) {
    Log::Fatal("SpatialSTKDistributor",
               "The SpatialSTKDistributor currently only works in conjunction "
               "with the UniformGridEnvironment");
  }
  if (!space) {
    Log::Fatal("SpatialSTKDistributor",
               "The SpatialSTKDistributor requires a DistributedSimSpace.");
  }
  box_length_ = space->GetInteractionRadius() * dparam->box_length_factor;

  whole_space_ = space->GetWholeSpace();
  // Calculate how many boxes fit along each dimension
  for (int i = 0; i < 3; i++) {
    int dimension_length = whole_space_[2 * i + 1] - whole_space_[2 * i];
    assert((dimension_length % box_length_ == 0) &&
           "The grid dimensions are not a multiple of its box length");
    num_boxes_axis_[i] = dimension_length / box_length_;
  }
  num_boxes_xy_ = num_boxes_axis_[0] * num_boxes_axis_[1];

  sstream << "generated:" << num_boxes_axis_[0] << "x" << num_boxes_axis_[1]
          << "x" << num_boxes_axis_[2];
  stk::io::fill_mesh(sstream.str(), bulk_);

  UpdateLocalSpace();
  UpdateNeighborRanks();
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
MathArray<int, 3> SpatialSTKDistributor::GetBoxCoordinates(
    size_t id) const {
  id--;
  MathArray<int, 3> box_coord;
  box_coord[2] = id / num_boxes_xy_;
  auto remainder = id % num_boxes_xy_;
  box_coord[1] = remainder / num_boxes_axis_[0];
  box_coord[0] = remainder % num_boxes_axis_[0];
  return box_coord;
}

// -----------------------------------------------------------------------------
void SpatialSTKDistributor::UpdateLocalSpace() {
  auto min = std::numeric_limits<int>::min();
  auto max = std::numeric_limits<int>::max();
  MathArray<int, 6> local_box_coord = {max, min, max, min, max, min};

  stk::mesh::EntityVector elements;
  stk::mesh::get_entities(bulk_, stk::topology::ELEM_RANK,
                          meta_.locally_owned_part(), elements);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for (const stk::mesh::Entity& element : elements) {
    auto id = bulk_.identifier(element);
    auto coord = GetBoxCoordinates(id);
    for (int i = 0; i < 3; ++i) {
      if (coord[i] < local_box_coord[2 * i]) {
        local_box_coord[2 * i] = coord[i];
      }
      if (coord[i] > local_box_coord[2 * i + 1]) {
        local_box_coord[2 * i + 1] = coord[i];
      }
    }
  }

  // since we are interested in the space enclosing all boxes add 1 in each
  // dimension
  local_box_coord[1]++;
  local_box_coord[3]++;
  local_box_coord[5]++;

  auto local_space = BoxToSpaceCoordinates(local_box_coord);
  auto* space = dynamic_cast<DistributedSimSpace*>(
      Simulation::GetActive()->GetSimulationSpace());
  if (!space) {
    Log::Fatal("SpatialSTKDistributor",
               "The SpatialSTKDistributor requires a DistributedSimSpace.");
  }
  space->SetLocalSpace(local_space);
}

void SpatialSTKDistributor::UpdateNeighborRanks() {
  neighbor_ranks_.clear();

  stk::mesh::EntityVector elements;
  stk::mesh::get_entities(bulk_, stk::topology::ELEM_RANK,
                          meta_.aura_part(), elements);

  for (const stk::mesh::Entity& element : elements) {
    auto neighbor = bulk_.parallel_owner_rank(element);
    neighbor_ranks_.insert(neighbor);  
  }
}

// -----------------------------------------------------------------------------
SimulationSpace::Space SpatialSTKDistributor::BoxToSpaceCoordinates(
    const MathArray<int, 6>& box_space) const {
  auto* space = dynamic_cast<DistributedSimSpace*>(
      Simulation::GetActive()->GetSimulationSpace());
  const auto& ws = space->GetWholeSpace();
  SimulationSpace::Space ret_val = box_space * box_length_;
  // add offset
  ret_val[0] += ws[0];
  ret_val[1] += ws[0];
  ret_val[2] += ws[2];
  ret_val[3] += ws[2];
  ret_val[4] += ws[4];
  ret_val[5] += ws[4];
  return ret_val;
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

// -----------------------------------------------------------------------------
void SpatialSTKDistributor::MigrateAgents() {
  std::unordered_map<int, std::vector<Agent*>> migrate_out;
  std::unordered_map<int, std::vector<Agent*>> migrate_in;
  int me;
  MPI_Comm_rank(MPI_COMM_WORLD, &me);

  auto fea = L2F([&](Agent* agent, AgentHandle) {
    auto box_coord = GetBoxCoordinates(agent->GetPosition());
    auto id = GetId(box_coord);
    auto entity = bulk_.get_entity(stk::topology::ELEM_RANK, id); 
    if (!entity.is_local_offset_valid()) {
      // FIXME documentation
      Log::Error("SpatialSTKDistributor::MigrateAgents", "Found an agent outside the local simulation space and the aura.");
    }
    auto owner = bulk_.parallel_owner_rank(entity);
    if (me != owner) {
      // migrate agent 
      std::cout << "migrate agent " << agent->GetPosition() << std::endl;
#pragma omp critical
      migrate_out[owner].push_back(agent);
    }
  });
  
  auto* rm = Simulation::GetActive()->GetResourceManager();
  auto* param = Simulation::GetActive()->GetParam();
  rm->ForEachAgentParallel(param->scheduling_batch_size, fea); 

  // send/receive agents
  SendReceive(MPI_COMM_WORLD, neighbor_ranks_, migrate_out, &migrate_in); 

  // remove agents from this rank
  // FIXME parallelize
  for (auto& el : migrate_out) {
    for (auto* agent : el.second) {
      rm->RemoveAgent(agent->GetUid());
    }
  }

  // add new agents
  // FIXME parallelize
  for (auto& el : migrate_in) {
    for (auto* agent : el.second) {
      rm->AddAgent(agent);
    }
  }
}

// -----------------------------------------------------------------------------
uint64_t SpatialSTKDistributor::GetId(const MathArray<int, 3>& box_coord) {
  return box_coord[2] * num_boxes_xy_ + box_coord[1] * num_boxes_axis_[0] + box_coord[0] + 1; 
}

// -----------------------------------------------------------------------------
MathArray<int, 3> SpatialSTKDistributor::GetBoxCoordinates(
    const Real3& position) const {
  // Check if conversion can be done without loosing information
  assert(floor(position[0]) <= std::numeric_limits<int32_t>::max());
  assert(floor(position[1]) <= std::numeric_limits<int32_t>::max());
  assert(floor(position[2]) <= std::numeric_limits<int32_t>::max());
  MathArray<int, 3> box_coord;
  box_coord[0] =
      (static_cast<int32_t>(floor(position[0])) - whole_space_[0]) /
      box_length_;
  box_coord[1] =
      (static_cast<int32_t>(floor(position[1])) - whole_space_[2]) /
      box_length_;
  box_coord[2] =
      (static_cast<int32_t>(floor(position[2])) - whole_space_[4]) /
      box_length_;
  return box_coord;
}


}  // namespace experimental
}  // namespace bdm

#else

#include "core/util/log.h"

namespace bdm {
namespace experimental {

Distributor::Distributor() {}
Distributor::~Distributor() {}

Distributor* CreateDistributor(DistributorType type) {
  Log::Fatal("CreateDistributor",
             "BioDynaMo was compiled without support for distributed "
             "execution.\nTo enable this features add the following argument "
             "to the cmake call: '-Ddse=on'");
  return nullptr;
}
  
// -----------------------------------------------------------------------------
void SpatialSTKDistributor::MigrateAgents() {
  Log::Fatal("CreateDistributor",
             "BioDynaMo was compiled without support for distributed "
             "execution.\nTo enable this features add the following argument "
             "to the cmake call: '-Ddse=on'");
}


}  // namespace experimental
}  // namespace bdm

#endif  // USE_DSE
