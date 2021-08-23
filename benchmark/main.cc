#include <benchmark/benchmark.h>
#include <memory>
#include "bdm_benchmark.h"

// BENCHMARK_MAIN();

int main(int argc, char** argv) {
  ::benchmark::BenchmarkReporter* rep = new ::benchmark::BdmJSONReporter();
  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks(NULL, rep);
}
