#include <../../build/benchmark/src/gbench/include/benchmark/benchmark.h>
#include "../../demo/tumor_concept/src/tumor_concept.h"
//#include "../../demo/soma_clustering/src/soma_clustering.h"


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