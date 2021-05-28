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

#ifdef USE_OPENCL
#include <vector>

#ifdef __APPLE__
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include "cl2.hpp"
#else
#define CL_HPP_TARGET_OPENCL_VERSION 210
#define CL_HPP_ENABLE_EXCEPTIONS
#include <CL/cl2.hpp>
#endif

#include "core/agent/cell.h"
#include "core/environment/environment.h"
#include "core/environment/uniform_grid_environment.h"
#include "core/gpu/opencl_state.h"
#include "core/operation/bound_space_op.h"
#include "core/operation/mechanical_forces_op_opencl.h"
#include "core/operation/operation.h"
#include "core/operation/operation_registry.h"
#include "core/shape.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"

namespace bdm {

void MechanicalForcesOpOpenCL::IsNonSphericalObjectPresent(const Agent* agent,
                                                           bool* answer) {
  if (agent->GetShape() != Shape::kSphere) {
    *answer = true;
  }
}

void MechanicalForcesOpOpenCL::operator()() {
  auto* sim = Simulation::GetActive();
  auto* grid = dynamic_cast<UniformGridEnvironment*>(sim->GetEnvironment());
  auto* param = sim->GetParam();
  auto* rm = sim->GetResourceManager();
  auto* ocl_state = sim->GetOpenCLState();

  if (!grid) {
    Log::Fatal(
        "MechanicalForcesOpOpenCL::operator()",
        "MechanicalForcesOpOpenCL only works with UniformGridEnvironement.");
  }

  // Check the number of NUMA domains on the system. Currently only 1 is
  // supported for GPU execution.
  if (ThreadInfo::GetInstance()->GetNumaNodes() > 1) {
    Log::Fatal("MechanicalForcesOpOpenCL",
               "\nThe GPU execution only supports systems with 1 NUMA domain.");
    return;
  }

  uint32_t num_objects = rm->GetNumAgents();

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
  std::array<cl_uint, 3> num_boxes_axis;
  cl_double squared_radius =
      grid->GetLargestAgentSize() * grid->GetLargestAgentSize();

  bool is_non_spherical_object = false;

  rm->ForEachAgent([&](Agent* agent, AgentHandle ah) {
    // Check if there are any non-spherical objects in our simulation, because
    // GPU accelerations currently supports only sphere-sphere interactions
    IsNonSphericalObjectPresent(agent, &is_non_spherical_object);
    if (is_non_spherical_object) {
      Log::Fatal("MechanicalForcesOpOpenCL",
                 "\nWe detected a non-spherical object during the GPU "
                 "execution. This is currently not supported.");
      return;
    }
    auto* cell = bdm_static_cast<Cell*>(agent);
    auto idx = ah.GetElementIdx();
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
    auto sh = AgentHandle(numa_node, i);
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
  grid->GetNumBoxesAxis(num_boxes_axis.data());

  // Allocate GPU buffers
  cl::Buffer positions_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                           num_objects * 3 * sizeof(cl_double),
                           cell_positions.data()->data());
  cl::Buffer diameters_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                           num_objects * sizeof(cl_double),
                           cell_diameters.data());
  cl::Buffer tractor_force_arg(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                               num_objects * 3 * sizeof(cl_double),
                               cell_tractor_force.data()->data());
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
  collide.setArg(6, param->simulation_time_step);
  collide.setArg(7, param->simulation_max_displacement);
  collide.setArg(8, squared_radius);

  collide.setArg(9, static_cast<cl_int>(num_objects));
  collide.setArg(10, starts_arg);
  collide.setArg(11, lengths_arg);
  collide.setArg(12, successors_arg);
  collide.setArg(13, nba_arg);
  collide.setArg(14, cell_movements_arg);

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
    Log::Error("MechanicalForcesOpOpenCL", err.what(), "(", err.err(),
               ") = ", ocl_state->GetErrorString(err.err()));
    throw;
  }

  try {
    queue->enqueueReadBuffer(cell_movements_arg, CL_TRUE, 0,
                             num_objects * 3 * sizeof(cl_double),
                             cell_movements.data()->data());
  } catch (const cl::Error& err) {
    Log::Error("MechanicalForcesOpOpenCL", err.what(), "(", err.err(),
               ") = ", ocl_state->GetErrorString(err.err()));
    throw;
  }

  // set new positions after all updates have been calculated
  // otherwise some cells would see neighbors with already updated positions
  // which would lead to inconsistencies
  rm->ForEachAgent([&](Agent* agent, AgentHandle ah) {
    auto* cell = dynamic_cast<Cell*>(agent);
    auto idx = ah.GetElementIdx();
    Double3 new_pos;
    new_pos[0] = cell_movements[idx][0];
    new_pos[1] = cell_movements[idx][1];
    new_pos[2] = cell_movements[idx][2];
    cell->UpdatePosition(new_pos);
    if (param->bound_space) {
      ApplyBoundingBox(agent, param->bound_space, param->min_bound,
                       param->max_bound);
    }
  });
}

}  // namespace bdm

#endif  // USE_OPENCL
