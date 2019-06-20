// -----------------------------------------------------------------------------
//
// Copyright (C) The BioDynaMo Project.
// All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_UTIL_TIMING_AGGREGATOR_H_
#define CORE_UTIL_TIMING_AGGREGATOR_H_

#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "core/util/math.h"

namespace bdm {

class TimingAggregator {
 public:
  TimingAggregator() {}
  ~TimingAggregator() {}

  void AddEntry(const std::string& key, int64_t value) {
    if (!timings_.count(key)) {
      std::vector<int64_t> data;
      data.push_back(value);
      timings_[key] = data;
    } else {
      timings_[key].push_back(value);
    }
  }

  void AddDescription(const std::string text) { descriptions_.push_back(text); }

 private:
  std::map<std::string, std::vector<int64_t>> timings_;
  std::vector<std::string> descriptions_;

  friend std::ostream& operator<<(std::ostream& os, const TimingAggregator& p);
};

inline std::ostream& operator<<(std::ostream& os, const TimingAggregator& ta) {
  os << std::endl;

  if (ta.timings_.size() != 0) {
    os << "\033[1mTotal execution time per operation:\033[0m" << std::endl;
    for (auto& timing : ta.timings_) {
      os << timing.first << ": "
         << std::accumulate(timing.second.begin(), timing.second.end(), 0)
         << std::endl;
    }
  } else {
    os << "No statistics were gathered!" << std::endl;
  }
  return os;
}
}  // namespace bdm

#endif  // CORE_UTIL_TIMING_AGGREGATOR_H_
