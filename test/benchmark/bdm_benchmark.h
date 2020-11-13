#ifndef BDM_BENCHMARK_H
#define BDM_BENCHMARK_H

#include <benchmark/benchmark.h>
#include "complexity.h"

namespace benchmark {

class BdmJSONReporter : public JSONReporter {
    void ReportRuns(std::vector<Run> const& reports);
    void PrintRunData(Run const& run);
};

}

#endif