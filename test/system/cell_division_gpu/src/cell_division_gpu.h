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
#include "core/util/math.h"

namespace bdm {

// ----------------------------------------------------------------------------
// Starting with 8 cells, we let each cell grow in volume up until a point
// a cell must divide. This tests whether the GPU accelerated mechanical
// interactions properly handle the creation of new cells.
// -----------------------------------------------------------------------------

inline void ExpectArrayNear(const std::array<double, 3>& actual,
                            const std::array<double, 3>& expected, bool* ret) {
  for (size_t i = 0; i < actual.size(); i++) {
    if (std::fabs(expected[i] - actual[i]) > 1e-9) {
      *ret = false;
      std::cout << "Wrong result! Expected " << expected[i]
                << ", but instead got " << actual[i]
                << ", which is a difference of "
                << std::fabs(expected[i] - actual[i])
                << ", which is larger than 1e-9" << std::endl;
    }
  }
}

enum ExecutionMode { kCpu, kCuda, kOpenCl };

inline void RunTest(bool* result, ExecutionMode mode) {
  auto set_param = [&](auto* param) {
    switch (mode) {
      case kCpu:
        break;
      case kOpenCl:
        param->use_opencl_ = true;
      case kCuda:
        param->use_gpu_ = true;
    }
  };

  Simulation simulation("cell_division_gpu", set_param);
  auto* rm = simulation.GetResourceManager();
  auto* param = simulation.GetParam();
  rm->Clear();

// We need to give every test the same seed for the RNG, because in the cell
// division, random numbers are used. Within a single executable these numbers
// vary. Also within the threads this needs to be enforced
#pragma omp parallel
  simulation.GetRandom()->SetSeed(1);

  size_t cells_per_dim = 2;
  auto construct = [](const std::array<double, 3>& position) {
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

  // Run for 10 timesteps. In step 2 a division should take place. In step 3
  // these new cells are instantiated
  simulation.GetScheduler()->Simulate(10);

  ExpectArrayNear(
      rm->GetSimObject(0)->GetPosition(),
      {4.1399071506916413909, -5.9871942139195297727, 2.8344890446256703065},
      result);
  ExpectArrayNear(
      rm->GetSimObject(1)->GetPosition(),
      {-2.4263219149482031511, -1.4202336557809887019, 29.769029317615839147},
      result);
  ExpectArrayNear(
      rm->GetSimObject(2)->GetPosition(),
      {-4.9118212650644856865, 23.156656083480623209, -9.1231684411316447125},
      result);
  ExpectArrayNear(
      rm->GetSimObject(3)->GetPosition(),
      {4.3076765979041251597, 15.615300607043293368, 25.657658447555828474},
      result);
  ExpectArrayNear(
      rm->GetSimObject(4)->GetPosition(),
      {28.139314619772036963, -0.20987998233654170388, 4.6381417441282613012},
      result);
  ExpectArrayNear(
      rm->GetSimObject(5)->GetPosition(),
      {24.417550786690171094, 3.347525366344008102, 28.067824703341415216},
      result);
  ExpectArrayNear(
      rm->GetSimObject(6)->GetPosition(),
      {16.614520566718258721, 15.828015607618416638, -4.8357284569095106974},
      result);
  ExpectArrayNear(
      rm->GetSimObject(7)->GetPosition(),
      {14.446017269290647889, 22.250832446808978204, 20.180438615017894932},
      result);
}

inline int Simulate(int argc, const char** argv) {
  bool result = true;

  // TODO(ahmad): after Trello card ("Fix inconsistency in cell state due to
  // direct updates in Biology Modules")
  // enable multithreading, and adjust results if necessary
  omp_set_num_threads(1);

  // Run CPU version
  RunTest(&result, kCpu);

  // Run GPU (CUDA) version
  RunTest(&result, kCuda);

  // Run GPU (OpenCL) version
  RunTest(&result, kOpenCl);

  return !result;
}

}  // namespace bdm

#endif  // SYSTEM_CELL_DIVISION_GPU_SRC_CELL_DIVISION_GPU_H_
