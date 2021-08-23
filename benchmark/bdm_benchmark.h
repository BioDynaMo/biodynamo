#ifndef BDM_BENCHMARK_H
#define BDM_BENCHMARK_H

#include <benchmark/benchmark.h>

namespace benchmark {

std::string GetBigOString(BigO complexity);

class BdmJSONReporter : public JSONReporter {
 public:
  BdmJSONReporter() : first_report_(true) {}
  virtual void ReportRuns(std::vector<Run> const& reports);

 private:
  virtual void PrintRunData(Run const& run);
  bool first_report_;
};

}  // namespace benchmark

#endif
