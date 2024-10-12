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

#include <benchmark/benchmark.h>
#include "pyramidal_cell.h"

namespace bdm {
namespace pyramidal_cell {

static void PyramidalCell1(benchmark::State& state) {
  const char* argv[3] = {
      "./pyramidal_cell1", "--inline-config",
      "{ \"bdm::Param\":{ \"export_visualization\": true } }"};
  for (auto _ : state) {
    Simulate(3, argv);
  }
}

BENCHMARK(PyramidalCell1)->MeasureProcessCPUTime();

static void PyramidalCell0(benchmark::State& state) {
  const char* argv[3] = {
      "./pyramidal_cell0", "--inline-config",
      "{ \"bdm::Param\":{ \"export_visualization\": false } }"};
  for (auto _ : state) {
    Simulate(3, argv);
  }
}

BENCHMARK(PyramidalCell0)->MeasureProcessCPUTime();

}  // namespace pyramidal_cell
}  // namespace bdm
