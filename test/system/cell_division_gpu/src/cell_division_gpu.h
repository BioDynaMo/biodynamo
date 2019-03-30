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

// 2. Define compile time parameter
BDM_CTPARAM() {
  BDM_CTPARAM_HEADER();

  BDM_CTPARAM_FOR(bdm, Cell) { using BiologyModules = CTList<GrowDivide>; };
};

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

  Simulation<> simulation("cell_division_gpu", set_param);
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
    Cell cell(position);
    cell.SetDiameter(30);
    cell.SetAdherence(0.4);
    cell.SetMass(1.0);
    cell.AddBiologyModule(GrowDivide(30.05, 5000, {gAllEventIds}));
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

  const auto* cells = rm->template Get<Cell>();

  ExpectArrayNear(
      (*cells)[0].GetPosition(),
      {-9.0797829452623730617, -0.79113441727480182664, -4.9688206724986825336},
      result);
  ExpectArrayNear(
      (*cells)[1].GetPosition(),
      {-6.8337437084335794779, -1.9204593423306242084, 27.826456463201008518},
      result);
  ExpectArrayNear(
      (*cells)[2].GetPosition(),
      {-7.4131468903093011846, 20.911807812004578722, 3.9335861673545626793},
      result);
  ExpectArrayNear(
      (*cells)[3].GetPosition(),
      {-8.033329329758023718, 17.243615257269006236, 23.532725100678181462},
      result);
  ExpectArrayNear(
      (*cells)[4].GetPosition(),
      {27.712113581281290209, -7.0863749219630483012, -0.078076864912935250995},
      result);
  ExpectArrayNear(
      (*cells)[5].GetPosition(),
      {25.484933246104343851, -4.0134835408100482113, 15.268471646405878062},
      result);
  ExpectArrayNear(
      (*cells)[6].GetPosition(),
      {16.920893247175463614, 28.834297997409986891, 0.49327324196877342377},
      result);
  ExpectArrayNear(
      (*cells)[7].GetPosition(),
      {25.408565278573540525, 28.704363681593566326, 20.869688211659408239},
      result);
}

inline int Simulate(int argc, const char** argv) {
  bool result = true;

  // TODO(ahmad): after Trello card ("Fix inconsistency in cell state due to
  // direct updates in Biology Modules")
  // enable multithreading, and adjust results if necessary
  omp_set_num_threads(1);

  // Run CPU version
  // Disabled because CPU is running a different type of execution context than
  // the GPU versions (update in place, instead of synchronously)
  // Can be reenabled once such a execution context is in place for CPU too
  // RunTest(&result, kCpu);

  // Run GPU (CUDA) version. We check the macro here because we don't want to
  // fatal in case CUDA is not installed
#ifdef USE_CUDA
  RunTest(&result, kCuda);
#endif

  // Run GPU (OpenCL) version. We check the macro here because we don't want to
  // fatal in case OpenCL is not installed
#ifdef USE_OPENCL
  RunTest(&result, kOpenCl);
#endif

  return !result;
}

}  // namespace bdm

#endif  // SYSTEM_CELL_DIVISION_GPU_SRC_CELL_DIVISION_GPU_H_
