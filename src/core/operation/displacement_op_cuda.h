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

#ifndef CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_
#define CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_

#include <vector>

#include "core/gpu/displacement_op_cuda_kernel.h"
#include "core/grid.h"
#include "core/operation/bound_space_op.h"
#include "core/resource_manager.h"
#include "core/shape.h"
#include "core/sim_object/cell.h"
#include "core/sim_object/so_handle.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"

namespace bdm {

inline void IsNonSphericalObjectPresent(const SimObject* so, bool* answer) {
  if (so->GetShape() != Shape::kSphere) {
    *answer = true;
  }
}

/// Defines the 3D physical interactions between physical objects
class DisplacementOpCuda {
 private:
  struct InitializeGPUData;
  struct UpdateCPUResults;

 public:
  DisplacementOpCuda() {}
  ~DisplacementOpCuda() {}

  void operator()() {
    auto* sim = Simulation::GetActive();
    auto* grid = sim->GetGrid();
    auto* param = sim->GetParam();
    auto* rm = sim->GetResourceManager();

    auto num_numa_nodes = ThreadInfo::GetInstance()->GetNumaNodes();
    std::vector<SoHandle::ElementIdx_t> offset(num_numa_nodes);
    offset[0] = 0;
    for (int nn = 1; nn < num_numa_nodes; nn++) {
      offset[nn] = offset[nn - 1] + rm->GetNumSimObjects(nn - 1);
    }

    uint32_t total_num_objects = rm->GetNumSimObjects();

    // Cannot use Double3 here, because the `data()` function returns a const
    // pointer to the underlying array, whereas the CUDA kernel will cast it to
    // a void pointer. The conversion of `const double *` to `void *` is
    // illegal.
    std::vector<std::array<double, 3>> cell_movements(total_num_objects);
    std::vector<SoHandle::ElementIdx_t> successors(total_num_objects);
    std::vector<uint32_t> starts;
    std::vector<uint16_t> lengths;
    std::vector<uint64_t> timestamps;
    uint32_t box_length;
    std::array<uint32_t, 3> num_boxes_axis;
    std::array<int32_t, 3> grid_dimensions;
    double squared_radius =
        grid->GetLargestObjectSize() * grid->GetLargestObjectSize();

    InitializeGPUData f(total_num_objects, offset);
    rm->ApplyOnAllElementsParallelDynamic(1000, f);

    // Populate successor list
    for (int i = 0; i < num_numa_nodes; i++) {
      for (size_t j = 0; j < rm->GetNumSimObjects(i); j++) {
        auto idx = offset[i] + j;
        successors[idx] =
            offset[i] + grid->successors_.data_[i][j].GetElementIdx();
      }
    }

    uint64_t current_timestamp = grid->timestamp_;
    starts.resize(grid->boxes_.size());
    lengths.resize(grid->boxes_.size());
    timestamps.resize(grid->boxes_.size());
    size_t i = 0;
    for (auto& box : grid->boxes_) {
      timestamps[i] = box.timestamp_;
      if (box.timestamp_ == current_timestamp) {
        lengths[i] = box.length_;
        starts[i] =
            offset[box.start_.GetNumaNode()] + box.start_.GetElementIdx();
      }
      i++;
    }
    grid->GetGridInfo(&box_length, &num_boxes_axis, &grid_dimensions);

    // If this is the first time we perform physics on GPU using CUDA
    if (cdo_ == nullptr) {
      // Allocate 25% more memory so we don't need to reallocate GPU memory
      // for every (small) change
      total_num_objects_ = static_cast<uint32_t>(1.25 * total_num_objects);
      num_boxes_ = static_cast<uint32_t>(1.25 * starts.size());

      // Allocate required GPU memory
      cdo_ = new DisplacementOpCudaKernel(total_num_objects_, num_boxes_);
    } else {
      // If the number of simulation objects increased
      if (total_num_objects >= total_num_objects_) {
        Log::Info("DisplacementOpCuda",
                  "\nThe number of cells increased signficantly (from ",
                  total_num_objects_, " to ", total_num_objects,
                  "), so we allocate bigger GPU buffers\n");
        total_num_objects_ = static_cast<uint32_t>(1.25 * total_num_objects);
        cdo_->ResizeCellBuffers(total_num_objects_);
      }

      // If the neighbor grid size increased
      if (starts.size() >= num_boxes_) {
        Log::Info("DisplacementOpCuda",
                  "\nThe number of boxes increased signficantly (from ",
                  num_boxes_, " to ", "), so we allocate bigger GPU buffers\n");
        num_boxes_ = static_cast<uint32_t>(1.25 * starts.size());
        cdo_->ResizeGridBuffers(num_boxes_);
      }
    }

    cdo_->LaunchDisplacementKernel(
        f.cell_positions.data()->data(), f.cell_diameters.data(),
        f.cell_tractor_force.data()->data(), f.cell_adherence.data(),
        f.cell_boxid.data(), f.mass.data(), &(param->simulation_time_step_),
        &(param->simulation_max_displacement_), &squared_radius,
        &total_num_objects, starts.data(), lengths.data(), timestamps.data(),
        &current_timestamp, successors.data(), &box_length,
        num_boxes_axis.data(), grid_dimensions.data(),
        cell_movements.data()->data());

    // set new positions after all updates have been calculated
    // otherwise some cells would see neighbors with already updated positions
    // which would lead to inconsistencies

    UpdateCPUResults b(cell_movements, offset);
    rm->ApplyOnAllElementsParallelDynamic(1000, b);
  }

 private:
  DisplacementOpCudaKernel* cdo_ = nullptr;
  uint32_t num_boxes_ = 0;
  uint32_t total_num_objects_ = 0;

  struct UpdateCPUResults : public Functor<void, SimObject*, SoHandle> {
    std::vector<std::array<double, 3>> cell_movements;
    std::vector<SoHandle::ElementIdx_t> offset;

    UpdateCPUResults(std::vector<std::array<double, 3>> cm,
                     const std::vector<SoHandle::ElementIdx_t>& offs) {
      cell_movements = cm;
      offset = offs;
    }

    void operator()(SimObject* so, SoHandle soh) override {
      auto* param = Simulation::GetActive()->GetParam();
      auto* cell = dynamic_cast<Cell*>(so);
      auto idx = offset[soh.GetNumaNode()] + soh.GetElementIdx();
      Double3 new_pos;
      new_pos[0] = cell_movements[idx][0];
      new_pos[1] = cell_movements[idx][1];
      new_pos[2] = cell_movements[idx][2];
      cell->UpdatePosition(new_pos);
      if (param->bound_space_) {
        ApplyBoundingBox(so, param->min_bound_, param->max_bound_);
      }
    }
  };

  struct InitializeGPUData : public Functor<void, SimObject*, SoHandle> {
    bool is_non_spherical_object = false;
    std::vector<Double3> cell_positions;
    std::vector<double> cell_diameters;
    std::vector<double> cell_adherence;
    std::vector<Double3> cell_tractor_force;
    std::vector<uint32_t> cell_boxid;
    std::vector<double> mass;
    std::vector<SoHandle::ElementIdx_t> offset;

    InitializeGPUData(uint32_t num_objects,
                      const std::vector<SoHandle::ElementIdx_t>& offs) {
      cell_positions.resize(num_objects);
      cell_diameters.resize(num_objects);
      cell_adherence.resize(num_objects);
      cell_tractor_force.resize(num_objects);
      cell_boxid.resize(num_objects);
      mass.resize(num_objects);
      offset = offs;
    }

    void operator()(SimObject* so, SoHandle soh) override {
      // Check if there are any non-spherical objects in our simulation, because
      // GPU accelerations currently supports only sphere-sphere interactions
      IsNonSphericalObjectPresent(so, &is_non_spherical_object);
      if (is_non_spherical_object) {
        Log::Fatal("DisplacementOpCuda",
                   "\nWe detected a non-spherical object during the GPU "
                   "execution. This is currently not supported.");
        return;
      }
      auto* cell = bdm_static_cast<Cell*>(so);
      auto idx = offset[soh.GetNumaNode()] + soh.GetElementIdx();
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

#endif  // CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_
