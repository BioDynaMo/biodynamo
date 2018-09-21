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
#include "simulation.h"
#include "timing.h"
#include "simulation_object.h"

namespace bdm {

static TimingAggregator gStatistics;

/// \brief Decorator for `Operations` to measure runtime
template <typename TOp>
struct OpTimer {
  explicit OpTimer(std::string timer_msg) : timer_msg_(timer_msg) {}
  explicit OpTimer(std::string timer_msg, const TOp& op)
      : timer_msg_(timer_msg), operation_(op) {}

  void operator()(SimulationObject* cells) {
    auto* param = Simulation::GetActive()->GetParam();
    if (param->statistics_) {
      Timing timer(timer_msg_, &gStatistics);
      operation_(cells);
    } else {
      operation_(cells);
    }
  }

  TOp* operator->() { return &operation_; }
  const TOp* operator->() const { return &operation_; }

 private:
  std::string timer_msg_;
  TOp operation_;
};

}  // namespace bdm

#endif  // OP_TIMER_H_
