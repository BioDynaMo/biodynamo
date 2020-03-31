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
#include "neuroscience/neurite_element.h"

namespace bdm {

using experimental::neuroscience::NeuriteElement;

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

    CountShapes cs;
    rm->ApplyOnAllElements(cs);

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
    std::vector<std::array<double, 3>> force_to_transmit_to_proximal_mass(
        total_num_objects);
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
        f.shape.data(), f.position.data()->data(), f.diameters.data(),
        f.cell_tractor_force.data()->data(), f.cell_adherence.data(),
        f.boxid.data(), f.mass.data(), f.ne_proximal_end.data()->data(),
        f.ne_distal_end.data()->data(), f.ne_axis.data()->data(),
        f.ne_tension.data(),
        f.ne_force_to_transmit_to_proximal_mass.data()->data(),
        f.daughter_left.data(), f.daughter_right.data(), f.mother.data(),
        f.has_daughter_or_mother.data(), &(param->simulation_time_step_),
        &(param->simulation_max_displacement_), &squared_radius,
        &total_num_objects, starts.data(), lengths.data(), timestamps.data(),
        &current_timestamp, successors.data(), &box_length,
        num_boxes_axis.data(), grid_dimensions.data(),
        cell_movements.data()->data(),
        force_to_transmit_to_proximal_mass.data()->data());

    // set new positions after all updates have been calculated
    // otherwise some cells would see neighbors with already updated positions
    // which would lead to inconsistencies

    UpdateCPUResults b(&cell_movements, offset);
    rm->ApplyOnAllElementsParallelDynamic(1000, b);
  }

 private:
  DisplacementOpCudaKernel* cdo_ = nullptr;
  uint32_t num_boxes_ = 0;
  uint32_t total_num_objects_ = 0;

  struct UpdateCPUResults : public Functor<void, SimObject*, SoHandle> {
    const std::vector<std::array<double, 3>>* cell_movements;
    const std::vector<std::array<double, 3>>*
        force_to_transmit_to_proximal_mass;
    std::vector<SoHandle::ElementIdx_t> offset;

    UpdateCPUResults(const std::vector<std::array<double, 3>>* cm,
                     const std::vector<SoHandle::ElementIdx_t>& offs) {
      cell_movements = cm;
      offset = offs;
    }

    void operator()(SimObject* so, SoHandle soh) override {
      auto* param = Simulation::GetActive()->GetParam();
      auto idx = offset[soh.GetNumaNode()] + soh.GetElementIdx();
      Double3 new_pos;
      new_pos[0] = (*cell_movements)[idx][0];
      new_pos[1] = (*cell_movements)[idx][1];
      new_pos[2] = (*cell_movements)[idx][2];
      so->ApplyDisplacement(new_pos);
      if (so->GetShape() == Shape::kCylinder) {
        auto ne = bdm_static_cast<NeuriteElement*>(so);
        Double3 temp;
        temp[0] = (*force_to_transmit_to_proximal_mass)[idx][0];
        temp[1] = (*force_to_transmit_to_proximal_mass)[idx][1];
        temp[2] = (*force_to_transmit_to_proximal_mass)[idx][2];
        ne->SetForceToTransmitToProximalMass(temp);
      }
      if (param->bound_space_) {
        ApplyBoundingBox(so, param->min_bound_, param->max_bound_);
      }
    }
  };

  struct CountShapes : public Functor<void, SimObject*, SoHandle> {
    uint32_t num_cylinders = 0;
    uint32_t num_spheres = 0;

    void operator()(SimObject* so, SoHandle soh) override {
      if (so->GetShape() == Shape::kSphere) {
        num_spheres++;
      } else {
        num_cylinders++;
      }
    }
  };

  struct InitializeGPUData : public Functor<void, SimObject*, SoHandle> {
    // Data members required for both Cell and NeuriteElement objects
    std::vector<Double3> position;
    std::vector<double> diameters;
    std::vector<double> mass;
    std::vector<uint32_t> boxid;
    std::vector<uint8_t> shape;

    // Cell-specific data members
    std::vector<double> cell_adherence;
    std::vector<Double3> cell_tractor_force;

    // Neurite-Element-specific data members
    std::vector<Double3> ne_distal_end;
    std::vector<Double3> ne_proximal_end;
    std::vector<Double3> ne_axis;
    std::vector<Double3> ne_force_to_transmit_to_proximal_mass;
    std::vector<double> ne_tension;
    std::vector<SoHandle::ElementIdx_t> daughter_left;
    std::vector<SoHandle::ElementIdx_t> daughter_right;
    std::vector<SoHandle::ElementIdx_t> mother;
    std::vector<uint8_t> has_daughter_or_mother;

    // Index offset required to merge NUMA-separated containers
    std::vector<SoHandle::ElementIdx_t> offset;

    InitializeGPUData(uint32_t num_objects,
                      const std::vector<SoHandle::ElementIdx_t>& offs) {
      diameters.resize(num_objects);
      mass.resize(num_objects);
      boxid.resize(num_objects);
      shape.resize(num_objects);
      position.resize(num_objects);
      cell_adherence.resize(num_objects);
      cell_tractor_force.resize(num_objects);
      ne_distal_end.resize(num_objects);
      ne_proximal_end.resize(num_objects);
      ne_axis.resize(num_objects);
      ne_force_to_transmit_to_proximal_mass.resize(num_objects);
      ne_tension.resize(num_objects);
      daughter_left.resize(num_objects);
      daughter_right.resize(num_objects);
      mother.resize(num_objects);
      has_daughter_or_mother.resize(num_objects);
      offset = offs;
    }

    void operator()(SimObject* so, SoHandle soh) override {
      auto idx = offset[soh.GetNumaNode()] + soh.GetElementIdx();
      auto* rm = Simulation::GetActive()->GetResourceManager();

      // FIXME: we can differentiate cells and neurite elements by shape
      // currently, but in the future this could change
      if (so->GetShape() == Shape::kSphere) {
        auto* cell = bdm_static_cast<Cell*>(so);
        shape[idx] = Shape::kSphere;
        position[idx] = cell->GetPosition();
        mass[idx] = cell->GetMass();
        cell_adherence[idx] = cell->GetAdherence();
        cell_tractor_force[idx] = cell->GetTractorForce();
      } else {
        auto* ne = bdm_static_cast<NeuriteElement*>(so);
        shape[idx] = Shape::kCylinder;
        position[idx] = ne->GetMassLocation();
        mass[idx] = ne->GetMass();
        ne_distal_end[idx] = ne->DistalEnd();
        ne_proximal_end[idx] = ne->ProximalEnd();
        ne_axis[idx] = ne->GetSpringAxis();
        ne_tension[idx] = ne->GetTension();
        ne_force_to_transmit_to_proximal_mass[idx] =
            ne->GetForceToTransmitToProximalMass();

        auto dl_ptr = ne->GetDaughterLeft();
        auto dr_ptr = ne->GetDaughterRight();
        auto m_ptr = ne->GetMother();

        if (dl_ptr) {
          has_daughter_or_mother[idx] |= kHasDaughterLeft;
          auto dl = rm->GetSoHandle(dl_ptr->GetUid());
          daughter_left[idx] = offset[dl.GetNumaNode()] + dl.GetElementIdx();
        }
        if (dr_ptr) {
          has_daughter_or_mother[idx] |= kHasDaughterRight;
          auto dr = rm->GetSoHandle(dr_ptr->GetUid());
          daughter_right[idx] = offset[dr.GetNumaNode()] + dr.GetElementIdx();
        }
        if (m_ptr) {
          has_daughter_or_mother[idx] |= kHasMother;
          auto m = rm->GetSoHandle(m_ptr->GetUid());
          mother[idx] = offset[m.GetNumaNode()] + m.GetElementIdx();
        }
      }
      diameters[idx] = so->GetDiameter();
      boxid[idx] = so->GetBoxIdx();
    }
  };
};

}  // namespace bdm

#endif  // CORE_OPERATION_DISPLACEMENT_OP_CUDA_H_
