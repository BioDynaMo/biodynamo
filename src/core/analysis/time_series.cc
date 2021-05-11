// -----------------------------------------------------------------------------
//
// Copyright (C) 2021 CERN & Newcastle University for the benefit of the
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

#include "core/analysis/time_series.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/util/log.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
TimeSeries& TimeSeries::operator=(TimeSeries&& other) {
  data_ = std::move(other.data_);
  return *this;
}

// -----------------------------------------------------------------------------
void TimeSeries::Add(const std::string& id, double (*f)(Simulation*)) {
  auto it = data_.find(id);
  if (it != data_.end()) {
    Log::Warning("TimeSeries::Add", "TimeSeries with id (", id,
                 ") exists already. Operation aborted.");
  }
  data_[id] = {f};
}

// -----------------------------------------------------------------------------
void TimeSeries::Update() {
  auto* sim = Simulation::GetActive();
  auto* scheduler = sim->GetScheduler();
  for (auto& entry : data_) {
    auto& result_data = entry.second;
    result_data.x_values.push_back(scheduler->GetSimulatedSteps());
    result_data.y_values.push_back(result_data.f(sim));
  }
}

// -----------------------------------------------------------------------------
const std::vector<double>& TimeSeries::GetXValues(const std::string& id) const {
  return data_.at(id).x_values;
}

// -----------------------------------------------------------------------------
const std::vector<double>& TimeSeries::GetYValues(const std::string& id) const {
  return data_.at(id).y_values;
}

}  // namespace experimental
}  // namespace bdm
