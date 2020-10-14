#include <benchmark.h>
#include "tumor_concept.h"

void BM_BDM_TUMOR(benchmark::State& state) {
  const char** argv = (const char**) malloc(2);
  argv[0] = "./demo";
  argv[1] = NULL;
  int argc = 1;
    for (auto _ : state)
      bdm::Simulate(argc, argv);
}
/*
int main(int argc, char** argv)
{
//  int j = 1;
//  if (argc == 2)
//    j = atoi(argv[1]);
  BENCHMARK(BM_BDM_TUMOR);
//  for (int i = 0; i < j; i++)
    ::benchmark::RunSpecifiedBenchmarks();
  return 0;
}*/