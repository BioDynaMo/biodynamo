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

#ifndef OP_TIMER_H_
#define OP_TIMER_H_

#include <string>
#include "timing.h"

namespace bdm {

using std::string;

static TimingAggregator gStatistics;

/// \brief Decorator for `Operations` to measure runtime
template <typename TOp>
struct OpTimer {
  explicit OpTimer(string timer_msg) : timer_msg_(timer_msg) {}
  explicit OpTimer(string timer_msg, const TOp& op)
      : timer_msg_(timer_msg), operation_(op) {}

  template <typename Container, typename TSimulation = Simulation<>>
  void operator()(Container* cells, uint16_t type_idx) {
    auto* param = TSimulation::GetActive()->GetParam();
    if (param->statistics_) {
      Timing timer(timer_msg_, &gStatistics);
      operation_(cells, type_idx);
    } else {
      operation_(cells, type_idx);
    }
  }

  TOp* operator->() { return &operation_; }
  const TOp* operator->() const { return &operation_; }

 private:
  string timer_msg_;
  TOp operation_;
};

}  // namespace bdm

#endif  // OP_TIMER_H_
