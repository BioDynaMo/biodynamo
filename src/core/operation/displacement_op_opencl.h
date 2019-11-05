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

#ifndef CORE_OPERATION_DISPLACEMENT_OP_OPENCL_H_
#define CORE_OPERATION_DISPLACEMENT_OP_OPENCL_H_

#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
#include <vector>

#include "core/gpu/opencl_state.h"
#include "core/grid.h"
#include "core/operation/bound_space_op.h"
#include "core/shape.h"
#include "core/sim_object/cell.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"

namespace bdm {

/// Defines the 3D physical interactions between physical objects
class DisplacementOpOpenCL {
 public:
  DisplacementOpOpenCL() {}
  ~DisplacementOpOpenCL() {}

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
          "DisplacementOpOpenCL",
          "\nThe GPU execution only supports systems with 1 NUMA domain.");
      return;
    }

    uint32_t num_objects = rm->GetNumSimObjects();

    auto* ocl_state = OpenCLState::GetInstance();
    auto context = ocl_state->GetOpenCLContext();
    auto queue = ocl_state->GetOpenCLCommandQueue();
    auto programs = ocl_state->GetOpenCLProgramList();

    // Cannot use Double3 here, because the `data()` function returns a const
    // pointer to the underlying array, whereas the CUDA kernal will cast it to
    // a void pointer. The conversion of `const double *` to `void *` is
    // illegal.
    std::vector<std::array<cl_double, 3>> cell_movements(num_objects);
    std::vector<std::array<cl_double, 3>> cell_positions(num_objects);
    std::vector<cl_double> cell_diameters(num_objects);
    std::vector<cl_double> cell_adherence(num_objects);
    std::vector<std::array<cl_double, 3>> cell_tractor_force(num_objects);
    std::vector<cl_uint> cell_boxid(num_objects);
    std::vector<cl_double> mass(num_objects);
    std::vector<cl_uint> starts;
    std::vector<cl_ushort> lengths;
    std::vector<cl_uint> successors(num_objects);
    cl_uint box_length;
    std::array<cl_uint, 3> num_boxes_axis;
    std::array<cl_int, 3> grid_dimensions;
    cl_double squared_radius =
        grid->GetLargestObjectSize() * grid->GetLargestObjectSize();

    bool is_non_spherical_object = false;

    rm->ApplyOnAllElements([&](SimObject* so, SoHandle soh) {
      // Check if there are any non-spherical objects in our simulation, because
      // GPU accelerations currently supports only sphere-sphere interactions
      IsNonSphericalObjectPresent(so, &is_non_spherical_object);
      if (is_non_spherical_object) {
        Log::Fatal("DisplacementOpOpenCL",
                   "\nWe detected a non-spherical object during the GPU "
                   "execution. This is currently not supported.");
        return;
      }
      auto* cell = bdm_static_cast<Cell*>(so);
      auto idx = soh.GetElementIdx();
      mass[idx] = cell->GetMass();
      cell_diameters[idx] = cell->GetDiameter();
      cell_adherence[idx] = cell->GetAdherence();
      cell_boxid[idx] = cell->GetBoxIdx();
      cell_tractor_force[idx][0] = cell->GetTractorForce()[0];
      cell_tractor_force[idx][1] = cell->GetTractorForce()[1];
      cell_tractor_force[idx][2] = cell->GetTractorForce()[2];
      cell_positions[idx][0] = cell->GetPosition()[0];
      cell_positions[idx][1] = cell->GetPosition()[1];
      cell_positions[idx][2] = cell->GetPosition()[2];
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

    // Allocate GPU buffers
    cl::Buffer positions_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                             num_objects * 3 * sizeof(cl_double),
                             cell_positions.data()->data());
    cl::Buffer diameters_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                             num_objects * sizeof(cl_double),
                             cell_diameters.data());
    cl::Buffer tractor_force_arg(
        *context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
        num_objects * 3 * sizeof(cl_double), cell_tractor_force.data()->data());
    cl::Buffer adherence_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                             num_objects * sizeof(cl_double),
                             cell_adherence.data());
    cl::Buffer box_id_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                          num_objects * sizeof(cl_uint), cell_boxid.data());
    cl::Buffer mass_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                        num_objects * sizeof(cl_double), mass.data());
    cl::Buffer cell_movements_arg(
        *context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
        num_objects * 3 * sizeof(cl_double), cell_movements.data()->data());
    cl::Buffer starts_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                          starts.size() * sizeof(cl_uint), starts.data());
    cl::Buffer lengths_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                           lengths.size() * sizeof(cl_short), lengths.data());
    cl::Buffer successors_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                              successors.size() * sizeof(cl_uint),
                              successors.data());
    cl::Buffer nba_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                       3 * sizeof(cl_uint), num_boxes_axis.data());
    cl::Buffer gd_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      3 * sizeof(cl_int), grid_dimensions.data());

    // Create the kernel object from our program
    // TODO(ahmad): generalize the program selection, in case we have more than
    // one. We can maintain an unordered map of programs maybe
    cl::Kernel collide((*programs)[0], "collide");

    // Set kernel parameters
    collide.setArg(0, positions_arg);
    collide.setArg(1, diameters_arg);
    collide.setArg(2, tractor_force_arg);
    collide.setArg(3, adherence_arg);
    collide.setArg(4, box_id_arg);
    collide.setArg(5, mass_arg);
    collide.setArg(6, param->simulation_time_step_);
    collide.setArg(7, param->simulation_max_displacement_);
    collide.setArg(8, squared_radius);

    collide.setArg(9, static_cast<cl_int>(num_objects));
    collide.setArg(10, starts_arg);
    collide.setArg(11, lengths_arg);
    collide.setArg(12, successors_arg);
    collide.setArg(13, box_length);
    collide.setArg(14, nba_arg);
    collide.setArg(15, gd_arg);
    collide.setArg(16, cell_movements_arg);

    // The amount of threads for each work group (similar to CUDA thread block)
    int block_size = 256;

    try {
      // The global size determines the total number of threads that will be
      // spawned on the GPU, in groups of local_size
      cl::NDRange global_size =
          cl::NDRange(num_objects + (block_size - (num_objects % block_size)));
      cl::NDRange local_size = cl::NDRange(block_size);
      queue->enqueueNDRangeKernel(collide, cl::NullRange, global_size,
                                  local_size);
    } catch (const cl::Error& err) {
      Log::Error("DisplacementOpOpenCL", err.what(), "(", err.err(), ") = ",
                 ocl_state->GetErrorString(err.err()));
      throw;
    }

    try {
      queue->enqueueReadBuffer(cell_movements_arg, CL_TRUE, 0,
                               num_objects * 3 * sizeof(cl_double),
                               cell_movements.data()->data());
    } catch (const cl::Error& err) {
      Log::Error("DisplacementOpOpenCL", err.what(), "(", err.err(), ") = ",
                 ocl_state->GetErrorString(err.err()));
      throw;
    }

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
};

}  // namespace bdm

#endif  // defined(USE_OPENCL) && !defined(__ROOTCLING__)

#endif  // CORE_OPERATION_DISPLACEMENT_OP_OPENCL_H_
