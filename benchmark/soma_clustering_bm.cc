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

#include <benchmark/benchmark.h>
#include "soma_clustering.h"

namespace bdm {
namespace soma_clustering {

static void SomaClustering1(benchmark::State& state) {
  const char* argv[3] = {
    "./soma_clustering1",
    "--inline-config",
    "{ \"bdm::Param\":{ \"export_visualization\": true } }"
  };
  for (auto _ : state) {
    Simulate(3, argv);
  }
}

BENCHMARK(SomaClustering1)->MeasureProcessCPUTime();

static void SomaClustering0(benchmark::State& state) {
  const char* argv[3] = {
    "./soma_clustering0",
    "--inline-config",
    "{ \"bdm::Param\":{ \"export_visualization\": false } }"
  };
  for (auto _ : state) {
    Simulate(3, argv);
  }
}

BENCHMARK(SomaClustering0)->MeasureProcessCPUTime();

}  // namespace soma_clustering
}  // namespace bdm
