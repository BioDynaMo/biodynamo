#include "bdm-bench.h"

int main(int argc, const char** argv)
{
  BENCHMARK(BM_BDM_SOMA);
  BENCHMARK(BM_BDM_TUMOR);
  ::benchmark::RunSpecifiedBenchmarks();
  return 0;
}