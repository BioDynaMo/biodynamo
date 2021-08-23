#include <benchmark/benchmark.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>  // for setprecision
#include <iostream>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

#include "bdm_benchmark.h"
#include "string_util.h"
#include "timers.h"

namespace benchmark {

namespace {

std::string StrEscape(const std::string& s) {
  std::string tmp;
  tmp.reserve(s.size());
  for (char c : s) {
    switch (c) {
      case '\b':
        tmp += "\\b";
        break;
      case '\f':
        tmp += "\\f";
        break;
      case '\n':
        tmp += "\\n";
        break;
      case '\r':
        tmp += "\\r";
        break;
      case '\t':
        tmp += "\\t";
        break;
      case '\\':
        tmp += "\\\\";
        break;
      case '"':
        tmp += "\\\"";
        break;
      default:
        tmp += c;
        break;
    }
  }
  return tmp;
}

std::string FormatKV(std::string const& key, std::string const& value) {
  return StrFormat("\"%s\": \"%s\"", StrEscape(key).c_str(),
                   StrEscape(value).c_str());
}

std::string FormatKV(std::string const& key, const char* value) {
  return StrFormat("\"%s\": \"%s\"", StrEscape(key).c_str(),
                   StrEscape(value).c_str());
}

std::string FormatKV(std::string const& key, bool value) {
  return StrFormat("\"%s\": %s", StrEscape(key).c_str(),
                   value ? "true" : "false");
}

std::string FormatKV(std::string const& key, int64_t value) {
  std::stringstream ss;
  ss << '"' << StrEscape(key) << "\": " << value;
  return ss.str();
}

std::string FormatKV(std::string const& key, IterationCount value) {
  std::stringstream ss;
  ss << '"' << StrEscape(key) << "\": " << value;
  return ss.str();
}

std::string FormatKV(std::string const& key, double value) {
  std::stringstream ss;
  ss << '"' << StrEscape(key) << "\": ";

  if (std::isnan(value))
    ss << (value < 0 ? "-" : "") << "NaN";
  else if (std::isinf(value))
    ss << (value < 0 ? "-" : "") << "Infinity";
  else {
    const auto max_digits10 =
        std::numeric_limits<decltype(value)>::max_digits10;
    const auto max_fractional_digits10 = max_digits10 - 1;
    ss << std::scientific << std::setprecision(max_fractional_digits10)
       << value;
  }
  return ss.str();
}

}  // end namespace

void BdmJSONReporter::ReportRuns(std::vector<Run> const& reports) {
  if (reports.empty()) {
    return;
  }
  std::string indent(4, ' ');
  std::ostream& out = GetOutputStream();
  if (!first_report_) {
    out << ",\n";
  }
  first_report_ = false;

  for (auto it = reports.begin(); it != reports.end(); ++it) {
    out << indent << "{\n";
    PrintRunData(*it);
    out << indent << '}';
    auto it_cp = it;
    if (++it_cp != reports.end()) {
      out << ",\n";
    }
  }
}

void BdmJSONReporter::PrintRunData(Run const& run) {
  std::string indent(6, ' ');
  std::ostream& out = GetOutputStream();
  out << indent << FormatKV("name", run.benchmark_name()) << ",\n";
  //  out << indent << FormatKV("memory", "100000") << ",\n";
  out << indent << FormatKV("run_name", run.run_name.str()) << ",\n";
  out << indent << FormatKV("run_type", [&run]() -> const char* {
    switch (run.run_type) {
      case BenchmarkReporter::Run::RT_Iteration:
        return "iteration";
      case BenchmarkReporter::Run::RT_Aggregate:
        return "aggregate";
    }
    BENCHMARK_UNREACHABLE();
  }()) << ",\n";
  out << indent << FormatKV("repetitions", run.repetitions) << ",\n";
  if (run.run_type != BenchmarkReporter::Run::RT_Aggregate) {
    out << indent << FormatKV("repetition_index", run.repetition_index)
        << ",\n";
  }
  out << indent << FormatKV("threads", run.threads) << ",\n";
  if (run.run_type == BenchmarkReporter::Run::RT_Aggregate) {
    out << indent << FormatKV("aggregate_name", run.aggregate_name) << ",\n";
  }
  if (run.error_occurred) {
    out << indent << FormatKV("error_occurred", run.error_occurred) << ",\n";
    out << indent << FormatKV("error_message", run.error_message) << ",\n";
  }
  if (!run.report_big_o && !run.report_rms) {
    out << indent << FormatKV("iterations", run.iterations) << ",\n";
    out << indent << FormatKV("memory", "10000") << ",\n";
    out << indent << FormatKV("real_time", run.GetAdjustedRealTime()) << ",\n";
    out << indent << FormatKV("cpu_time", run.GetAdjustedCPUTime());
    out << ",\n"
        << indent << FormatKV("time_unit", GetTimeUnitString(run.time_unit));
  } else if (run.report_big_o) {
    out << indent << FormatKV("cpu_coefficient", run.GetAdjustedCPUTime())
        << ",\n";
    out << indent << FormatKV("real_coefficient", run.GetAdjustedRealTime())
        << ",\n";
    out << indent << FormatKV("big_o", GetBigOString(run.complexity)) << ",\n";
    out << indent << FormatKV("time_unit", GetTimeUnitString(run.time_unit));
  } else if (run.report_rms) {
    out << indent << FormatKV("rms", run.GetAdjustedCPUTime());
  }

  for (auto& c : run.counters) {
    out << ",\n" << indent << FormatKV(c.first, c.second);
  }

  if (run.has_memory_result) {
    out << ",\n" << indent << FormatKV("allocs_per_iter", run.allocs_per_iter);
    out << ",\n" << indent << FormatKV("max_bytes_used", run.max_bytes_used);
  }

  if (!run.report_label.empty()) {
    out << ",\n" << indent << FormatKV("label", run.report_label);
  }
  out << '\n';
};

}  // namespace benchmark
