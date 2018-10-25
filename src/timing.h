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

#ifndef TIMING_H_
#define TIMING_H_

#include <chrono>
#include <iostream>
#include <string>

#include "param.h"
#include "timing_aggregator.h"

namespace bdm {

static TimingAggregator gStatistics;

class Timing {
 public:
  typedef std::chrono::high_resolution_clock Clock;

  static int64_t Timestamp() {
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    auto time = Clock::now();
    auto since_epoch = time.time_since_epoch();
    auto millis = duration_cast<milliseconds>(since_epoch);
    return millis.count();
  }

  template <typename TFunctor, typename TSimulation = Simulation<>>
  static void Time(const std::string& description, TFunctor&& f) {
    static bool use_timer_ = TSimulation::GetActive()->GetParam()->statistics_;
    if (use_timer_) {
      Timing timing(description, &gStatistics);
      f();
    } else {
      f();
    }
  }

  explicit Timing(const std::string& description = "")
      : start_{Timestamp()}, text_{description} {}

  Timing(const std::string& description, TimingAggregator* aggregator)
      : start_{Timestamp()}, text_{description}, aggregator_{aggregator} {}

  ~Timing() {
    int64_t duration = (Timestamp() - start_);
    if (aggregator_ == nullptr) {
      std::cout << text_ << " " << duration << " ms" << std::endl;
    } else {
      aggregator_->AddEntry(text_, duration);
    }
  }

 private:
  int64_t start_;
  std::string text_;
  TimingAggregator* aggregator_ = nullptr;
};

}  // namespace bdm

#endif  // TIMING_H_
