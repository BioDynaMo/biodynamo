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

void TumorConcept(benchmark::State& state) {
  // FIXME memory leak
  const char** argv = (const char**)malloc(2);
  argv[0] = "./demo";
  argv[1] = NULL;
  int argc = 1;
  for (auto _ : state) {
    Simulate(argc, argv);
  }
}

BENCHMARK(TumorConcept);

}  // namespace tumor_concept
}  // namespace bdm
