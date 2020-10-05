#include <benchmark/benchmark.h>

static void BM_StringCreation(benchmark::State& state) {
  for (auto _ : state)
    std::string empty_string;
}
// Register the function as a benchmark
BENCHMARK(BM_StringCreation);

// Define another benchmark
static void BM_StringCopy(benchmark::State& state) {
  std::string x = "hello";
  for (auto _ : state)
    std::string copy(x);
}
BENCHMARK(BM_StringCopy);

BENCHMARK_MAIN();

/*
#include "../../demo/tumor_concept/src/tumor_concept.h"

static void BM_BDM(benchmark::State& state) {
  const char** argv = (const char**) malloc(2);
  argv[0] = "./tumor_concept";
  argv[1] = NULL;
  int argc = 1;
    for (auto _ : state)
      bdm::Simulate(argc, argv);
}

int main(int argc, const char** argv)
{
  BENCHMARK(BM_BDM);
  ::benchmark::RunSpecifiedBenchmarks();
  return 0;
}
*/