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
#include "tumor_concept.h"

namespace bdm {
namespace tumor_concept {

static void TumorConcept1(benchmark::State& state) {
  // FIXME memory leak
  const char** argv = (const char**)malloc(4*sizeof(char*));
  argv[0] = "./tumor_concept";
  argv[1] = "--inline-config";
  argv[2] = "{ \"bdm::Param\":{ \"export_visualization_\": true } }";
  argv[3] = NULL;
  int argc = 3;
  for (auto _ : state) {
    Simulate(argc, argv);
  }
}

BENCHMARK(TumorConcept1)->MeasureProcessCPUTime();

static void TumorConcept0(benchmark::State& state) {
  // FIXME memory leak
  const char** argv = (const char**)malloc(4*sizeof(char*));
  argv[0] = "./tumor_concept";
  argv[1] = "--inline-config";
  argv[2] = "{ \"bdm::Param\":{ \"export_visualization_\": false } }";
  argv[3] = NULL;
  int argc = 3;
  for (auto _ : state) {
    Simulate(argc, argv);
  }
}

BENCHMARK(TumorConcept0)->MeasureProcessCPUTime();

}  // namespace tumor_concept
}  // namespace bdm
