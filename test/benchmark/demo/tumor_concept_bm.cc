#include <benchmark.h>
#include "tumor_concept.h"

void TumorConcept(benchmark::State& state) {
  const char** argv = (const char**) malloc(2);
  argv[0] = "./demo";
  argv[1] = NULL;
  int argc = 1;
    for (auto _ : state) {
      bdm::Simulate(argc, argv);
    }
}

BENCHMARK(TumorConcept);
BENCHMARK_MAIN();

