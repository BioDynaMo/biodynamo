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
#include "core/operation/bound_space_op.h"
#include "core/resource_manager.h"
#include "core/shape.h"
#include "core/sim_object/cell.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects
class DisplacementOpCuda {
 public:
  DisplacementOpCuda() {}
  ~DisplacementOpCuda() {}

  void IsNonSphericalObjectPresent(const SimObject* so, bool* answer) {
    if (so->GetShape() != Shape::kSphere) {
      *answer = true;
    }
  }

  void operator()() {
    auto* sim = Simulation::GetActive();
    auto* grid = sim->GetGrid();
    auto* param = sim->GetParam();
    auto* rm = sim->GetResourceManager();

    // Check the number of NUMA domains on the system. Currently only 1 is
    // supported for GPU execution.
    if (ThreadInfo::GetInstance()->GetNumaNodes() > 1) {
      Log::Fatal(
          "DisplacementOpCuda",
          "\nThe GPU execution only supports systems with 1 NUMA domain.");
      return;
    }

    uint32_t num_objects = rm->GetNumSimObjects();

    // Cannot use Double3 here, because the `data()` function returns a const
    // pointer to the underlying array, whereas the CUDA kernal will cast it to
    // a void pointer. The conversion of `const double *` to `void *` is
    // illegal.
    std::vector<std::array<double, 3>> cell_movements(num_objects);
    std::vector<Double3> cell_positions(num_objects);
    std::vector<double> cell_diameters(num_objects);
    std::vector<double> cell_adherence(num_objects);
    std::vector<Double3> cell_tractor_force(num_objects);
    std::vector<uint32_t> cell_boxid(num_objects);
    std::vector<double> mass(num_objects);
    std::vector<uint32_t> starts;
    std::vector<uint16_t> lengths;
    std::vector<uint32_t> successors(num_objects);
    uint32_t box_length;
    std::array<uint32_t, 3> num_boxes_axis;
    std::array<int32_t, 3> grid_dimensions;
    double squared_radius =
        grid->GetLargestObjectSize() * grid->GetLargestObjectSize();

    bool is_non_spherical_object = false;

    rm->ApplyOnAllElements([&](SimObject* so, SoHandle soh) {
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
      auto idx = soh.GetElementIdx();
      mass[idx] = cell->GetMass();
      cell_diameters[idx] = cell->GetDiameter();
      cell_adherence[idx] = cell->GetAdherence();
      cell_tractor_force[idx] = cell->GetTractorForce();
      cell_positions[idx] = cell->GetPosition();
      cell_boxid[idx] = cell->GetBoxIdx();
    });

    uint16_t numa_node = 0;  // GPU code only supports 1 NUMA domain currently
    for (size_t i = 0; i < grid->successors_.size(numa_node); i++) {
      auto sh = SoHandle(numa_node, i);
      successors[i] = grid->successors_[sh].GetElementIdx();
    }

    starts.resize(grid->boxes_.size());
    lengths.resize(grid->boxes_.size());
    size_t i = 0;
    for (auto& box : grid->boxes_) {
      starts[i] = box.start_.GetElementIdx();
      lengths[i] = box.length_;
      i++;
    }
    grid->GetGridInfo(&box_length, &num_boxes_axis, &grid_dimensions);

    // If this is the first time we perform physics on GPU using CUDA
    if (cdo_ == nullptr) {
      // Allocate 25% more memory so we don't need to reallocate GPU memory
      // for every (small) change
      uint32_t new_num_objects = static_cast<uint32_t>(1.25 * num_objects);
      uint32_t new_num_boxes = static_cast<uint32_t>(1.25 * starts.size());

      // Store these extended buffer sizes for future reference
      num_objects_ = new_num_objects;
      num_boxes_ = new_num_boxes;

      // Allocate required GPU memory
      cdo_ = new DisplacementOpCudaKernel(new_num_objects, new_num_boxes);
    } else {
      // If the number of simulation objects increased
      if (num_objects >= num_objects_) {
        Log::Info("DisplacementOpCuda",
                  "\nThe number of cells increased signficantly (from ",
                  num_objects_, " to ", num_objects,
                  "), so we allocate bigger GPU buffers\n");
        uint32_t new_num_objects = static_cast<uint32_t>(1.25 * num_objects);
        num_objects_ = new_num_objects;
        cdo_->ResizeCellBuffers(new_num_objects);
      }

      // If the neighbor grid size increased
      if (starts.size() >= num_boxes_) {
        Log::Info("DisplacementOpCuda",
                  "\nThe number of boxes increased signficantly (from ",
                  num_boxes_, " to ", "), so we allocate bigger GPU buffers\n");
        uint32_t new_num_boxes = static_cast<uint32_t>(1.25 * starts.size());
        num_boxes_ = new_num_boxes;
        cdo_->ResizeGridBuffers(new_num_boxes);
      }
    }

    cdo_->LaunchDisplacementKernel(
        cell_positions.data()->data(), cell_diameters.data(),
        cell_tractor_force.data()->data(), cell_adherence.data(),
        cell_boxid.data(), mass.data(), &(param->simulation_time_step_),
        &(param->simulation_max_displacement_), &squared_radius, &num_objects,
        starts.data(), lengths.data(), successors.data(), &box_length,
        num_boxes_axis.data(), grid_dimensions.data(),
        cell_movements.data()->data());

    // set new positions after all updates have been calculated
    // otherwise some cells would see neighbors with already updated positions
    // which would lead to inconsistencies
    rm->ApplyOnAllElements([&](SimObject* so, SoHandle soh) {
      auto* cell = dynamic_cast<Cell*>(so);
      auto idx = soh.GetElementIdx();
      Double3 new_pos;
      new_pos[0] = cell_movements[idx][0];
      new_pos[1] = cell_movements[idx][1];
      new_pos[2] = cell_movements[idx][2];
      cell->UpdatePosition(new_pos);
      if (param->bound_space_) {
        ApplyBoundingBox(so, param->min_bound_, param->max_bound_);
      }
    });
  }

 private:
  DisplacementOpCudaKernel* cdo_ = nullptr;
  uint32_t num_boxes_ = 0;
  uint32_t num_objects_ = 0;
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_
