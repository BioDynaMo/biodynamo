#include <benchmark.h>
#include "soma_clustering.h"

static void BM_BDM(benchmark::State& state) {
  const char** argv = (const char**) malloc(2);
  argv[0] = "./demo";
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