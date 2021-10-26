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

#ifndef SYSTEM_CELL_DIVISION_GPU_SRC_CELL_DIVISION_GPU_H_
#define SYSTEM_CELL_DIVISION_GPU_SRC_CELL_DIVISION_GPU_H_

#include <array>

#include "biodynamo.h"
#include "core/param/command_line_options.h"
#include "core/util/math.h"
#include "core/util/timing.h"

namespace bdm {

// ----------------------------------------------------------------------------
// Starting with 8 cells, we let each cell grow in volume up until a point
// a cell must divide. This tests whether the GPU accelerated mechanical
// interactions properly handle the creation of new cells.
// -----------------------------------------------------------------------------

inline void ExpectArrayNear(const Double3& actual, const Double3& expected,
                            bool* wrong) {
  for (size_t i = 0; i < actual.size(); i++) {
    if (std::fabs(expected[i] - actual[i]) > 1e-9) {
      *wrong = true;
      std::cout << "Wrong result! Expected " << expected[i]
                << ", but instead got " << actual[i]
                << ", which is a difference of "
                << std::fabs(expected[i] - actual[i])
                << ", which is larger than 1e-9" << std::endl;
    }
  }
}

inline void RunTest(bool* wrong, OpComputeTarget mode, uint64_t timesteps,
                    uint64_t cells_per_dim) {
  std::cout << "Running simulation on ";
  auto set_param = [&](auto* param) {
    switch (mode) {
      case kCpu:
        std::cout << "CPU (" << omp_get_max_threads() << " threads)\n";
        break;
      case kOpenCl:
        std::cout << "GPU (OpenCL)\n";
        param->compute_target = "opencl";
        break;
      case kCuda:
        std::cout << "GPU (CUDA)\n";
        param->compute_target = "cuda";
        break;
    }
  };

  Simulation simulation("cell_division_gpu", set_param);
  auto* rm = simulation.GetResourceManager();
  rm->ClearAgents();

// We need to give every test the same seed for the RNG, because in the cell
// division, random numbers are used. Within a single executable these numbers
// vary. Also within the threads this needs to be enforced
#pragma omp parallel
  simulation.GetRandom()->SetSeed(1);

  auto construct = [](const Double3& position) {
    auto* cell = new Cell(position);
    cell->SetDiameter(30);
    cell->SetAdherence(0.4);
    cell->SetMass(1.0);
    cell->AddBehavior(new GrowthDivision(30.05, 5000));
    return cell;
  };

  for (size_t x = 0; x < cells_per_dim; x++) {
    double x_pos = x * 20.0;
    for (size_t y = 0; y < cells_per_dim; y++) {
      double y_pos = y * 20.0;
      for (size_t z = 0; z < cells_per_dim; z++) {
        auto new_simulation_object = construct({x_pos, y_pos, z * 20.0});
        rm->AddAgent(new_simulation_object);
      }
    }
  }

  {
    Timing timer("Execution time");
    simulation.GetScheduler()->Simulate(timesteps);
  }

  // TODO: add verification of results
}

inline int Simulate(int argc, const char** argv) {
  auto options = CommandLineOptions(argc, argv);
  options.AddOption<bool>("verify", "false");
  options.AddOption<uint64_t>("cells-per-dim", "64");
  options.AddOption<uint64_t>("timesteps", "5");

  uint64_t cells_per_dim = options.Get<uint64_t>("cells-per-dim");
  uint64_t timesteps = options.Get<uint64_t>("timesteps");

  bool wrong = true;
  bool is_opencl = options.Get<bool>("opencl");
  bool is_cuda = options.Get<bool>("cuda");

  // TODO(ahmad): after Trello card ("Fix inconsistency in cell state due to
  // direct updates in Biology Modules") enable multithreading, and adjust
  // results if necessary
  // omp_set_num_threads(1);

  if (!is_cuda && !is_opencl) {
    // Run CPU version
    RunTest(&wrong, kCpu, timesteps, cells_per_dim);
  }

#ifdef USE_CUDA
  if (is_cuda) {
    // Run GPU (CUDA) version
    RunTest(&wrong, kCuda, timesteps, cells_per_dim);
  }
#endif  // USE_CUDA

#ifdef USE_OPENCL
  if (is_opencl) {
    // Run GPU (OpenCL) version
    RunTest(&wrong, kOpenCl, timesteps, cells_per_dim);
  }
#endif  // USE_OPENCL

  return !wrong;
}

}  // namespace bdm

#endif  // SYSTEM_CELL_DIVISION_GPU_SRC_CELL_DIVISION_GPU_H_
