#include <benchmark.h>
#include "soma_clustering.h"

void BM_BDM_SOMA(benchmark::State& state) {
  const char** argv = (const char**) malloc(2);
  argv[0] = "./demo";
  argv[1] = NULL;
  int argc = 1;
    for (auto _ : state)
      bdm::Simulate(argc, argv);
}

// int main(int argc, char** argv)
// {
//   BENCHMARK(BM_BDM_SOMA);
//   ::benchmark::RunSpecifiedBenchmarks();
//   ::benchmark::Initialize(&argc, argv);
//   return 0;
// }