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

#ifndef CORE_OPERATION_MECHANICAL_FORCES_OP_CUDA_H_
#define CORE_OPERATION_MECHANICAL_FORCES_OP_CUDA_H_

#include <vector>

#include "core/agent/agent_handle.h"
#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "core/environment/uniform_grid_environment.h"
#include "core/gpu/mechanical_forces_op_cuda_kernel.h"
#include "core/operation/bound_space_op.h"
#include "core/operation/operation_registry.h"
#include "core/resource_manager.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"
#include "core/util/vtune_helper.h"

namespace bdm {

inline void IsNonSphericalObjectPresent(const Agent* agent, bool* answer) {
  if (agent->GetShape() != Shape::kSphere) {
    *answer = true;
  }
}

/// Defines the 3D physical interactions between physical objects
struct MechanicalForcesOpCuda : public StandaloneOperationImpl {
  BDM_OP_HEADER(MechanicalForcesOpCuda);

 private:
  struct InitializeGPUData;
  struct UpdateCPUResults;

 public:
  void SetUp() override {
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

    uint32_t total_num_objects = rm->GetNumAgents();

    i_ = new InitializeGPUData(total_num_objects, offset);
    rm->ForEachAgentParallel(1000, *i_);

    // Populate successor list
    for (int i = 0; i < num_numa_nodes; i++) {
      for (size_t j = 0; j < rm->GetNumAgents(i); j++) {
        auto idx = offset[i] + j;
        i_->successors[idx] =
            offset[grid->successors_.data_[i][j].GetNumaNode()] +
            grid->successors_.data_[i][j].GetElementIdx();
      }
    }

    auto num_boxes = grid->boxes_.size();
    i_->current_timestamp = grid->timestamp_;
    i_->starts.reserve(num_boxes);
    i_->lengths.reserve(num_boxes);
    i_->timestamps.reserve(num_boxes);
    size_t i = 0;
    for (auto& box : grid->boxes_) {
      i_->timestamps[i] = box.timestamp_;
      if (box.timestamp_ == i_->current_timestamp) {
        i_->lengths[i] = box.length_;
        i_->starts[i] =
            offset[box.start_.GetNumaNode()] + box.start_.GetElementIdx();
      }
      i++;
    }
    grid->GetGridInfo(&(i_->box_length), &(i_->num_boxes_axis),
                      &(i_->grid_dimensions));
  }

  void operator()() override {
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
    cdo_->LaunchMechanicalForcesKernel(
        i_->cell_positions.data()->data(), i_->cell_diameters.data(),
        i_->cell_tractor_force.data()->data(), i_->cell_adherence.data(),
        i_->cell_boxid.data(), i_->mass.data(), &(param->simulation_time_step),
        &(param->simulation_max_displacement), &squared_radius,
        &total_num_objects, i_->starts.data(), i_->lengths.data(),
        i_->timestamps.data(), &(i_->current_timestamp), i_->successors.data(),
        &(i_->box_length), i_->num_boxes_axis.data(),
        i_->grid_dimensions.data(), i_->cell_movements.data()->data());
  }

  void TearDown() override {
    auto u = UpdateCPUResults(&(i_->cell_movements), i_->offset);
    Simulation::GetActive()->GetResourceManager()->ForEachAgentParallel(1000,
                                                                        u);
  }

 private:
  MechanicalForcesOpCudaKernel* cdo_ = nullptr;
  InitializeGPUData* i_ = nullptr;
  uint32_t num_boxes_ = 0;
  uint32_t total_num_objects_ = 0;

  struct UpdateCPUResults : public Functor<void, Agent*, AgentHandle> {
    std::vector<std::array<double, 3>>* cell_movements = nullptr;
    std::vector<AgentHandle::ElementIdx_t> offset;

    UpdateCPUResults(std::vector<std::array<double, 3>>* cm,
                     const std::vector<AgentHandle::ElementIdx_t>& offs) {
      cell_movements = cm;
      offset = offs;
    }

    void operator()(Agent* agent, AgentHandle ah) override {
      auto* param = Simulation::GetActive()->GetParam();
      auto* cell = dynamic_cast<Cell*>(agent);
      auto idx = offset[ah.GetNumaNode()] + ah.GetElementIdx();
      Double3 new_pos;
      new_pos[0] = (*cell_movements)[idx][0];
      new_pos[1] = (*cell_movements)[idx][1];
      new_pos[2] = (*cell_movements)[idx][2];
      cell->UpdatePosition(new_pos);
      if (param->bound_space) {
        ApplyBoundingBox(agent, param->min_bound, param->max_bound);
      }
    }
  };

  struct InitializeGPUData : public Functor<void, Agent*, AgentHandle> {
    bool is_non_spherical_object = false;
    // Cannot use Double3 here, because the `data()` function returns a const
    // pointer to the underlying array, whereas the CUDA kernel will cast it to
    // a void pointer. The conversion of `const double *` to `void *` is
    // illegal.
    std::vector<std::array<double, 3>> cell_movements;
    std::vector<Double3> cell_positions;
    std::vector<double> cell_diameters;
    std::vector<double> cell_adherence;
    std::vector<Double3> cell_tractor_force;
    std::vector<uint32_t> cell_boxid;
    std::vector<double> mass;
    std::vector<AgentHandle::ElementIdx_t> offset;
    std::vector<AgentHandle::ElementIdx_t> successors;
    std::vector<uint32_t> starts;
    std::vector<uint16_t> lengths;
    std::vector<uint64_t> timestamps;
    uint64_t current_timestamp;
    uint32_t box_length;
    std::array<uint32_t, 3> num_boxes_axis;
    std::array<int32_t, 3> grid_dimensions;

    InitializeGPUData(uint32_t num_objects,
                      const std::vector<AgentHandle::ElementIdx_t>& offs) {
      cell_movements.reserve(num_objects);
      cell_positions.reserve(num_objects);
      successors.reserve(num_objects);
      cell_diameters.reserve(num_objects);
      cell_adherence.reserve(num_objects);
      cell_tractor_force.reserve(num_objects);
      cell_boxid.reserve(num_objects);
      mass.reserve(num_objects);
      offset = offs;
    }

    void operator()(Agent* agent, AgentHandle ah) override {
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
      mass[idx] = cell->GetMass();
      cell_diameters[idx] = cell->GetDiameter();
      cell_adherence[idx] = cell->GetAdherence();
      cell_tractor_force[idx] = cell->GetTractorForce();
      cell_positions[idx] = cell->GetPosition();
      cell_boxid[idx] = cell->GetBoxIdx();
    }
  };
};

}  // namespace bdm

#endif  // CORE_OPERATION_MECHANICAL_FORCES_OP_CUDA_H_
