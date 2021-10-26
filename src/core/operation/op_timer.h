// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & University of Surrey for the benefit of the
// BioDynaMo collaboration. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
//
// See the LICENSE file distributed with this work for details.
// See the NOTICE file distributed with this work for additional information
// regarding copyright ownership.
//
// -----------------------------------------------------------------------------

#ifndef CORE_OPERATION_OP_TIMER_H_
#define CORE_OPERATION_OP_TIMER_H_

#include <string>
#include <utility>
#include "core/simulation.h"
#include "core/util/timing.h"

namespace bdm {

/// \brief Decorator for `Operations` to measure runtime
template <typename TOp>
class OpTimer {
 public:
  explicit OpTimer(std::string timer_msg) : timer_msg_(std::move(timer_msg)) {}
  explicit OpTimer(std::string timer_msg, const TOp& op)
      : timer_msg_(std::move(timer_msg)), operation_(op) {}

  template <typename Container>
  void operator()(Container* cells, uint16_t numa_node, uint16_t type_idx) {
    auto* param = Simulation::GetActive()->GetParam();
    auto* agg = Simulation::GetActive()->GetScheduler()->GetOpTimes();
    if (param->statistics) {
      Timing timer(timer_msg_, agg);
      operation_(cells, numa_node, type_idx);
    } else {
      operation_(cells, numa_node, type_idx);
    }
  }

  TOp* operator->() { return &operation_; }
  const TOp* operator->() const { return &operation_; }

 private:
  std::string timer_msg_;
  TOp operation_;
};

}  // namespace bdm

#endif  // CORE_OPERATION_OP_TIMER_H_
