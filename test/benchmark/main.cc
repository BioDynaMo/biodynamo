#include <benchmark.h>
#include <memory>
#include "bdm_benchmark.h"

//BENCHMARK_MAIN();

int main(int argc, char** argv) {
    //typedef std::unique_ptr<benchmark::BenchmarkReporter> PtrType;

    ::benchmark::BenchmarkReporter* rep = new ::benchmark::BdmJSONReporter();
    //PtrType rep = PtrType(new benchmark::BdmJSONReporter);

    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks(rep);
//    ::benchmark::RunSpecifiedBenchmarks(new ::benchmark::BdmJSONReporter);
}
