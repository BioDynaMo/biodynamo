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

#ifndef CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_
#define CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_

#include <vector>

#include "core/gpu/displacement_op_cuda_kernel.h"
#include "core/operation/bound_space_op.h"
#include "core/resource_manager.h"
#include "core/shape.h"
#include "core/simulation.h"
#include "core/util/log.h"
#include "core/util/type.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects
template <typename TSimulation = Simulation>
class DisplacementOpCuda {
 public:
  DisplacementOpCuda() {}
  ~DisplacementOpCuda() {}

  template <typename TContainer>
  typename std::enable_if<is_soa_sphere<TContainer>::value>::type operator()(
      TContainer* cells, uint16_t numa_node, uint16_t type_idx) {
    auto* sim = Simulation::GetActive();
    auto* grid = sim->GetGrid();
    auto* param = sim->GetParam();

    std::vector<std::array<double, 3>> cell_movements(cells->size());
    std::vector<double> mass(cells->size());
    std::vector<uint32_t> starts;
    std::vector<uint16_t> lengths;
    std::vector<uint32_t> successors(cells->size());
    uint32_t box_length;
    uint32_t num_objects = cells->size();
    std::array<uint32_t, 3> num_boxes_axis;
    std::array<int32_t, 3> grid_dimensions;
    double squared_radius =
        grid->GetLargestObjectSize() * grid->GetLargestObjectSize();

    // We need to create a mass vector, because it is not stored by default in
    // a cell container
    cells->FillMassVector(&mass);
    grid->GetSuccessors(&successors);
    grid->GetBoxInfo(&starts, &lengths);
    grid->GetGridInfo(&box_length, &num_boxes_axis, &grid_dimensions);

    // If this is the first time we perform physics on GPU using CUDA
    if (cdo_ == nullptr) {
      // Allocate 25% more memory so we don't need to reallocate GPU memory
      // for every (small) change
      uint32_t new_num_objects = static_cast<uint32_t>(1.25 * num_objects);
      uint32_t new_num_boxes = static_cast<uint32_t>(1.25 * starts.size());

      // Store these extende buffer sizes for future reference
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
        cells->GetPositionPtr(), cells->GetDiameterPtr(),
        cells->GetTractorForcePtr(), cells->GetAdherencePtr(),
        cells->GetBoxIdPtr(), mass.data(), &(param->simulation_time_step_),
        &(param->simulation_max_displacement_), &squared_radius, &num_objects,
        starts.data(), lengths.data(), successors.data(), &box_length,
        num_boxes_axis.data(), grid_dimensions.data(),
        cell_movements.data()->data());

// set new positions after all updates have been calculated
// otherwise some cells would see neighbors with already updated positions
// which would lead to inconsistencies
#pragma omp parallel for
    for (size_t i = 0; i < cells->size(); i++) {
      auto&& cell = (*cells)[i];
      cell.UpdatePosition(cell_movements[i]);
      if (param->bound_space_) {
        ApplyBoundingBox(&cell, param->min_bound_, param->max_bound_);
      }
      cell.SetPosition(cell.GetPosition());

      // Reset biological movement to 0.
      cell.SetTractorForce({0, 0, 0});
    }
  }

  template <typename TContainer>
  typename std::enable_if<!is_soa_sphere<TContainer>::value>::type operator()(
      TContainer* cells, uint16_t numa_node, uint16_t type_idx) {
    Fatal("DisplacementOpCuda",
          "You tried to compile GPU-specific function calls for a non-SOA data "
          "structure or non-spherical simulation object.");
  }

 private:
  DisplacementOpCudaKernel* cdo_ = nullptr;
  uint32_t num_boxes_ = 0;
  uint32_t num_objects_ = 0;
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_
