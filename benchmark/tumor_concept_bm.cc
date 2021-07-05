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
#include "tumor_concept.h"

namespace bdm {
namespace tumor_concept {

static void TumorConcept1(benchmark::State& state) {
  // FIXME memory leak
 const char* argv[3] = {
    "./tumor_concept1",
    "--inline-config",
    "{ \"bdm::Param\":{ \"export_visualization\": true } }"
  };
  for (auto _ : state) {
    Simulate(3, argv);
  }
}

BENCHMARK(TumorConcept1)->MeasureProcessCPUTime();

static void TumorConcept0(benchmark::State& state) {
  // FIXME memory leak
  const char* argv[3] = {
    "./tumor_concept0",
    "--inline-config",
    "{ \"bdm::Param\":{ \"export_visualization\": false } }"
  };
  for (auto _ : state) {
    Simulate(3, argv);
  }
}

BENCHMARK(TumorConcept0)->MeasureProcessCPUTime();

}  // namespace tumor_concept
}  // namespace bdm
