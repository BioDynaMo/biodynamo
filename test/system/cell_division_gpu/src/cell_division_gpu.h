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

enum ExecutionMode { kCpu, kCuda, kOpenCl };

inline void RunTest(bool* wrong, ExecutionMode mode, bool verify,
                    uint64_t timesteps, uint64_t cells_per_dim) {
  std::cout << "Running simulation on ";
  auto set_param = [&](auto* param) {
    switch (mode) {
      case kCpu:
        std::cout << "CPU (single core)\n";
        break;
      case kOpenCl:
        std::cout << "GPU (OpenCL)\n";
        param->use_gpu_ = true;
        param->use_opencl_ = true;
        break;
      case kCuda:
        std::cout << "GPU (CUDA)\n";
        param->use_gpu_ = true;
        break;
    }
  };

  // Results below are with 2 cells per dimension and after 10 timesteps
  if (verify) {
    cells_per_dim = 2;
    timesteps = 10;
  }

  Simulation simulation("cell_division_gpu", set_param);
  auto* rm = simulation.GetResourceManager();
  rm->Clear();

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
    cell->AddBiologyModule(new GrowDivide(30.05, 5000, {gAllEventIds}));
    return cell;
  };

  for (size_t x = 0; x < cells_per_dim; x++) {
    double x_pos = x * 20.0;
    for (size_t y = 0; y < cells_per_dim; y++) {
      double y_pos = y * 20.0;
      for (size_t z = 0; z < cells_per_dim; z++) {
        auto new_simulation_object = construct({x_pos, y_pos, z * 20.0});
        rm->push_back(new_simulation_object);
      }
    }
  }

  {
    Timing timer("Execution time");
    simulation.GetScheduler()->Simulate(timesteps);
  }

  if (verify) {
    ExpectArrayNear(
        rm->GetSimObject(0)->GetPosition(),
        {4.1399071506916413909, -5.9871942139195297727, 2.8344890446256703065},
        wrong);
    ExpectArrayNear(
        rm->GetSimObject(1)->GetPosition(),
        {-2.4263219149482031511, -1.4202336557809887019, 29.769029317615839147},
        wrong);
    ExpectArrayNear(
        rm->GetSimObject(2)->GetPosition(),
        {-4.9118212650644856865, 23.156656083480623209, -9.1231684411316447125},
        wrong);
    ExpectArrayNear(
        rm->GetSimObject(3)->GetPosition(),
        {4.3076765979041251597, 15.615300607043293368, 25.657658447555828474},
        wrong);
    ExpectArrayNear(
        rm->GetSimObject(4)->GetPosition(),
        {28.139314619772036963, -0.20987998233654170388, 4.6381417441282613012},
        wrong);
    ExpectArrayNear(
        rm->GetSimObject(5)->GetPosition(),
        {24.417550786690171094, 3.347525366344008102, 28.067824703341415216},
        wrong);
    ExpectArrayNear(
        rm->GetSimObject(6)->GetPosition(),
        {16.614520566718258721, 15.828015607618416638, -4.8357284569095106974},
        wrong);
    ExpectArrayNear(
        rm->GetSimObject(7)->GetPosition(),
        {14.446017269290647889, 22.250832446808978204, 20.180438615017894932},
        wrong);
    if (!wrong) {
      std::cout << "Simulation was verified correctly!" << std::endl;
    }
  }
}

inline int Simulate(int argc, const char** argv) {
  auto parser = CustomCLOParser(argc, argv);
  bool cuda = parser.GetValue("cuda", false);
  bool opencl = parser.GetValue("opencl", false);
  bool verify = parser.GetValue("verify", false);
  uint64_t cells_per_dim = parser.GetValue("cells-per-dim", 2);
  uint64_t timesteps = parser.GetValue("timesteps", 10);

  bool wrong = true;

  // TODO(ahmad): after Trello card ("Fix inconsistency in cell state due to
  // direct updates in Biology Modules") enable multithreading, and adjust
  // results if necessary
  omp_set_num_threads(1);

  if (!cuda && !opencl) {
    // Run CPU version
    RunTest(&wrong, kCpu, verify, timesteps, cells_per_dim);
  }

#ifdef USE_CUDA
  if (cuda) {
    // Run GPU (CUDA) version
    RunTest(&wrong, kCuda, verify, timesteps, cells_per_dim);
  }
#endif  // USE_CUDA

#ifdef USE_OPENCL
  if (opencl) {
    // Run GPU (OpenCL) version
    RunTest(&wrong, kOpenCl, verify, timesteps, cells_per_dim);
  }
#endif  // USE_OPENCL

  return !wrong;
}

}  // namespace bdm

#endif  // SYSTEM_CELL_DIVISION_GPU_SRC_CELL_DIVISION_GPU_H_
