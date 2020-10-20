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

#include <benchmark.h>
#include "soma_clustering.h"

namespace bdm {
namespace soma_clustering {

static void SomaClustering1(benchmark::State& state) {
  // FIXME memory leak
  const char** argv = (const char**)malloc(4*sizeof(char*));
  argv[0] = "./soma_clustering";
  argv[1] = "--inline-config";
  argv[2] = "{ \"bdm::Param\":{ \"export_visualization_\": true } }";
  argv[3] = NULL;
  int argc = 3;
  for (auto _ : state) {
    Simulate(argc, argv);
  }
}

BENCHMARK(SomaClustering1)->MeasureProcessCPUTime();

static void SomaClustering0(benchmark::State& state) {
  // FIXME memory leak
  const char** argv = (const char**)malloc(4*sizeof(char*));
  argv[0] = "./soma_clustering_0";
  argv[1] = "--inline-config";
  argv[2] = "{ \"bdm::Param\":{ \"export_visualization_\": false } }";
  argv[3] = NULL;
  int argc = 3;
  for (auto _ : state) {
    Simulate(argc, argv);
  }
}

BENCHMARK(SomaClustering0)->MeasureProcessCPUTime();


}  // namespace soma_clustering
}  // namespace bdm
