// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include "core/operation/mechanical_forces_op_cuda.h"
#include <vector>

#include "core/agent/agent_handle.h"
#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "core/environment/uniform_grid_environment.h"
#include "core/gpu/cuda_pinned_memory.h"
#include "core/operation/bound_space_op.h"
#include "core/resource_manager.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/thread_info.h"
#include "core/util/timing.h"
#include "core/util/type.h"
#include "core/util/vtune_helper.h"

namespace bdm {

// -----------------------------------------------------------------------------
void IsNonSphericalObjectPresent(const Agent* agent, bool* answer) {
  if (agent->GetShape() != Shape::kSphere) {
    *answer = true;
  }
}

namespace detail {

// -----------------------------------------------------------------------------
struct InitializeGPUData : public Functor<void, Agent*, AgentHandle> {
  bool is_non_spherical_object = false;

  double* cell_movements = nullptr;
  double* cell_positions = nullptr;
  double* cell_diameters = nullptr;
  double* cell_adherence = nullptr;
  double* cell_tractor_force = nullptr;
  uint32_t* cell_boxid = nullptr;
  double* mass = nullptr;
  uint32_t* successors = nullptr;

  std::vector<AgentHandle::ElementIdx_t> offset;

  uint32_t* starts = nullptr;
  uint16_t* lengths = nullptr;
  uint64_t* timestamps = nullptr;
  uint64_t* current_timestamp = nullptr;
  uint32_t* num_boxes_axis = nullptr;
  UniformGridEnvironment* grid = nullptr;

  uint64_t allocated_num_objects = 0;
  uint64_t allocated_num_boxes = 0;

  InitializeGPUData();

  virtual ~InitializeGPUData();

  void Initialize(uint64_t num_objects, uint64_t num_boxes,
                  const std::vector<AgentHandle::ElementIdx_t>& offs,
                  UniformGridEnvironment* g);

  void operator()(Agent* agent, AgentHandle ah) override;

 private:
  void FreeAgentBuffers();

  void FreeGridBuffers();
};

// -----------------------------------------------------------------------------
InitializeGPUData::InitializeGPUData() {}

// -----------------------------------------------------------------------------
InitializeGPUData::~InitializeGPUData() {
  if (current_timestamp != nullptr) {
    CudaFreePinned(current_timestamp);
    CudaFreePinned(num_boxes_axis);
  }

  if (allocated_num_objects != 0) {
    FreeAgentBuffers();
  }

  if (allocated_num_boxes != 0) {
    FreeGridBuffers();
  }
}

// -----------------------------------------------------------------------------
void InitializeGPUData::Initialize(
    uint64_t num_objects, uint64_t num_boxes,
    const std::vector<AgentHandle::ElementIdx_t>& offs,
    UniformGridEnvironment* g) {
  if (current_timestamp == nullptr) {
    CudaAllocPinned(&current_timestamp, 1);
    CudaAllocPinned(&num_boxes_axis, 3);
  }

  if (allocated_num_objects < num_objects) {
    if (allocated_num_objects != 0) {
      FreeAgentBuffers();
    }
    allocated_num_objects = num_objects * 1.25;
    CudaAllocPinned(&cell_movements, allocated_num_objects * 3);
    CudaAllocPinned(&cell_positions, allocated_num_objects * 3);
    CudaAllocPinned(&cell_diameters, allocated_num_objects);
    CudaAllocPinned(&cell_adherence, allocated_num_objects);
    CudaAllocPinned(&cell_tractor_force, allocated_num_objects * 3);
    CudaAllocPinned(&cell_boxid, allocated_num_objects);
    CudaAllocPinned(&mass, allocated_num_objects);
    CudaAllocPinned(&successors, allocated_num_objects);
  }

  if (allocated_num_boxes < num_boxes) {
    if (allocated_num_boxes != 0) {
      FreeGridBuffers();
    }
    allocated_num_boxes = num_boxes * 1.25;
    CudaAllocPinned(&starts, allocated_num_boxes);
    CudaAllocPinned(&lengths, allocated_num_boxes);
    CudaAllocPinned(&timestamps, allocated_num_boxes);
  }

  offset = offs;
  grid = g;
}

// -----------------------------------------------------------------------------
void InitializeGPUData::FreeAgentBuffers() {
  CudaFreePinned(cell_movements);
  CudaFreePinned(cell_positions);
  CudaFreePinned(cell_diameters);
  CudaFreePinned(cell_adherence);
  CudaFreePinned(cell_tractor_force);
  CudaFreePinned(cell_boxid);
  CudaFreePinned(mass);
  CudaFreePinned(successors);
}

// -----------------------------------------------------------------------------
void InitializeGPUData::FreeGridBuffers() {
  CudaFreePinned(starts);
  CudaFreePinned(lengths);
  CudaFreePinned(timestamps);
}

// -----------------------------------------------------------------------------
void InitializeGPUData::operator()(Agent* agent, AgentHandle ah) {
  // Check if there are any non-spherical objects in our simulation, because
  // GPU accelerations currently supports only sphere-sphere interactions
  IsNonSphericalObjectPresent(agent, &is_non_spherical_object);
  if (is_non_spherical_object) {
    Log::Fatal("MechanicalForcesOpCuda",
               "\nWe detected a non-spherical object during the GPU "
               "execution. This is currently not supported.");
    return;
  }
  auto* cell = bdm_static_cast<Cell*>(agent);
  auto idx = offset[ah.GetNumaNode()] + ah.GetElementIdx();
  auto idxt3 = idx * 3;
  mass[idx] = cell->GetMass();
  cell_diameters[idx] = cell->GetDiameter();
  cell_adherence[idx] = cell->GetAdherence();
  const auto& tf = cell->GetTractorForce();
  const auto& pos = cell->GetPosition();

  for (uint64_t i = 0; i < 3; ++i) {
    cell_tractor_force[idxt3 + i] = tf[i];
    cell_positions[idxt3 + i] = pos[i];
  }

  cell_boxid[idx] = cell->GetBoxIdx();

  // populate successor list
  auto nid = ah.GetNumaNode();
  auto el = ah.GetElementIdx();
  successors[idx] = offset[grid->successors_.data_[nid][el].GetNumaNode()] +
                    grid->successors_.data_[nid][el].GetElementIdx();
}

}  // namespace detail

// -----------------------------------------------------------------------------
struct UpdateCPUResults : public Functor<void, Agent*, AgentHandle> {
  double* cell_movements = nullptr;
  std::vector<AgentHandle::ElementIdx_t> offset;

  UpdateCPUResults(double* cm,
                   const std::vector<AgentHandle::ElementIdx_t>& offs);
  virtual ~UpdateCPUResults();

  void operator()(Agent* agent, AgentHandle ah) override;
};

// -----------------------------------------------------------------------------
UpdateCPUResults::UpdateCPUResults(
    double* cm, const std::vector<AgentHandle::ElementIdx_t>& offs) {
  cell_movements = cm;
  offset = offs;
}

// -----------------------------------------------------------------------------
UpdateCPUResults::~UpdateCPUResults() {}

// -----------------------------------------------------------------------------
void UpdateCPUResults::operator()(Agent* agent, AgentHandle ah) {
  auto* param = Simulation::GetActive()->GetParam();
  auto* cell = bdm_static_cast<Cell*>(agent);
  auto idx = offset[ah.GetNumaNode()] + ah.GetElementIdx();
  idx *= 3;
  Double3 new_pos = {cell_movements[idx], cell_movements[idx + 1],
                     cell_movements[idx + 2]};
  cell->UpdatePosition(new_pos);
  if (param->bound_space) {
    ApplyBoundingBox(agent, param->min_bound, param->max_bound);
  }
}

// -----------------------------------------------------------------------------
void MechanicalForcesOpCuda::SetUp() {
  // Timing timer("MechanicalForcesOpCuda::SetUp");
  auto* sim = Simulation::GetActive();
  auto* grid = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
  auto* rm = sim->GetResourceManager();

  if (!grid) {
    Log::Fatal(
        "MechanicalForcesOpCuda::operator()",
        "MechanicalForcesOpCuda only works with UniformGridEnvironement.");
  }

  auto num_numa_nodes = ThreadInfo::GetInstance()->GetNumaNodes();
  std::vector<AgentHandle::ElementIdx_t> offset(num_numa_nodes);
  offset[0] = 0;
  for (int nn = 1; nn < num_numa_nodes; nn++) {
    offset[nn] = offset[nn - 1] + rm->GetNumAgents(nn - 1);
  }

  auto total_num_objects = rm->GetNumAgents();
  auto num_boxes = grid->boxes_.size();

  if (i_ == nullptr) {
    i_ = new detail::InitializeGPUData();
  }
  i_->Initialize(total_num_objects, num_boxes, offset, grid);
  {
    // Timing timer("MechanicalForcesOpCuda::toColumnar");
    rm->ForEachAgentParallel(1000, *i_);
  }

  *i_->current_timestamp = grid->timestamp_;
#pragma omp parallel for
  for (uint64_t i = 0; i < grid->boxes_.size(); ++i) {
    auto& box = grid->boxes_[i];
    i_->timestamps[i] = box.timestamp_;
    if (box.timestamp_ == *i_->current_timestamp) {
      i_->lengths[i] = box.length_;
      i_->starts[i] =
          offset[box.start_.GetNumaNode()] + box.start_.GetElementIdx();
    }
  }
  grid->GetNumBoxesAxis(i_->num_boxes_axis);
}

// -----------------------------------------------------------------------------
void MechanicalForcesOpCuda::operator()() {
  auto* sim = Simulation::GetActive();
  auto* grid = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
  auto* param = sim->GetParam();
  auto* rm = sim->GetResourceManager();

  uint32_t total_num_objects = rm->GetNumAgents();
  auto num_boxes = grid->boxes_.size();
  // If this is the first time we perform physics on GPU using CUDA
  if (cdo_ == nullptr) {
    // Allocate 25% more memory so we don't need to reallocate GPU memory
    // for every (small) change
    total_num_objects_ = static_cast<uint32_t>(1.25 * total_num_objects);
    num_boxes_ = static_cast<uint32_t>(1.25 * num_boxes);

    // Allocate required GPU memory
    cdo_ = new MechanicalForcesOpCudaKernel(total_num_objects_, num_boxes_);
  } else {
    // If the number of agents increased
    if (total_num_objects >= total_num_objects_) {
      Log::Info("MechanicalForcesOpCuda",
                "\nThe number of cells increased signficantly (from ",
                total_num_objects_, " to ", total_num_objects,
                "), agent we allocate bigger GPU buffers\n");
      total_num_objects_ = static_cast<uint32_t>(1.25 * total_num_objects);
      cdo_->ResizeCellBuffers(total_num_objects_);
    }

    // If the neighbor grid size increased
    if (num_boxes >= num_boxes_) {
      Log::Info("MechanicalForcesOpCuda",
                "\nThe number of boxes increased signficantly (from ",
                num_boxes_, " to ", "), so we allocate bigger GPU buffers\n");
      num_boxes_ = static_cast<uint32_t>(1.25 * num_boxes);
      cdo_->ResizeGridBuffers(num_boxes_);
    }
  }

  double squared_radius =
      grid->GetLargestObjectSize() * grid->GetLargestObjectSize();

  // Timing timer("MechanicalForcesOpCuda::Kernel");
  cdo_->LaunchMechanicalForcesKernel(
      i_->cell_positions, i_->cell_diameters, i_->cell_tractor_force,
      i_->cell_adherence, i_->cell_boxid, i_->mass,
      &(param->simulation_time_step), &(param->simulation_max_displacement),
      &squared_radius, &total_num_objects, i_->starts, i_->lengths,
      i_->timestamps, i_->current_timestamp, i_->successors, i_->num_boxes_axis,
      i_->cell_movements);
}

// -----------------------------------------------------------------------------
void MechanicalForcesOpCuda::TearDown() {
  cdo_->Sync();
  // Timing timer("MechanicalForcesOpCuda::TearDown");
  auto u = UpdateCPUResults(i_->cell_movements, i_->offset);
  Simulation::GetActive()->GetResourceManager()->ForEachAgentParallel(1000, u);
}

}  // namespace bdm
