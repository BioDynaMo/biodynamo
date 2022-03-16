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

#include "core/analysis/time_series.h"
#include <TBufferJSON.h>
#include <iostream>
#include "core/analysis/reduce.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/util/io.h"
#include "core/util/log.h"

namespace bdm {
namespace experimental {

// -----------------------------------------------------------------------------
TimeSeries::Data::Data() = default;

// -----------------------------------------------------------------------------
TimeSeries::Data::Data(double (*ycollector)(Simulation*),
                       double (*xcollector)(Simulation*))
    : ycollector(ycollector), xcollector(xcollector) {}

// -----------------------------------------------------------------------------
TimeSeries::Data::Data(Reducer<double>* y_reducer_collector,
                       double (*xcollector)(Simulation*))
    : y_reducer_collector(y_reducer_collector), xcollector(xcollector) {}

// -----------------------------------------------------------------------------
TimeSeries::Data::Data(const Data& other)
    : ycollector(other.ycollector),
      xcollector(other.xcollector),
      x_values(other.x_values),
      y_values(other.y_values),
      y_error_low(other.y_error_low),
      y_error_high(other.y_error_high) {
  if (other.y_reducer_collector) {
    y_reducer_collector = other.y_reducer_collector->NewCopy();
  }
}

// -----------------------------------------------------------------------------
TimeSeries::Data& TimeSeries::Data::operator=(const Data& other) {
  if (this == &other) {
    return *this;
  }
  if (other.y_reducer_collector) {
    y_reducer_collector = other.y_reducer_collector->NewCopy();
  }
  ycollector = other.ycollector;
  xcollector = other.xcollector;
  x_values = other.x_values;
  y_values = other.y_values;
  y_error_low = other.y_error_low;
  y_error_high = other.y_error_high;
  return *this;
}

// -----------------------------------------------------------------------------
TimeSeries::Data::~Data() {
  if (y_reducer_collector) {
    delete y_reducer_collector;
  }
}

// -----------------------------------------------------------------------------
double TimeSeries::ComputeError(const TimeSeries& ts1, const TimeSeries& ts2) {
  // verify that all TimeSeries contain the same entries
  if (ts1.data_.size() != ts2.data_.size()) {
    Log::Warning("TimeSeries::ComputeError",
                 "The time series objects that should be merged do not "
                 "contain the same entries. Operation aborted.");
    return std::numeric_limits<double>::infinity();
  }
  for (auto& p : ts1.data_) {
    if (ts2.data_.find(p.first) == ts2.data_.end()) {
      Log::Warning("TimeSeries::ComputeError",
                   "The time series objects that should be merged do not "
                   "contain the same entries. Operation aborted");
      return std::numeric_limits<double>::infinity();
    }
  }

  for (auto& pair : ts1.data_) {
    auto& key = pair.first;
    // check that all time series objects have the same x values
    auto& xref = ts1.data_.at(key).x_values;
    auto& xcurrent = ts2.data_.at(key).x_values;
    if (xref.size() != xcurrent.size()) {
      Log::Warning("TimeSeries::ComputeError",
                   "The time series objects for entry (", key,
                   ") have different x_values. Operation aborted.");
      return std::numeric_limits<double>::infinity();
    } else {
      for (uint64_t j = 0; j < xref.size(); ++j) {
        if (std::abs(xref[j] - xcurrent[j]) > 1e-5) {
          Log::Warning("TimeSeries::ComputeError",
                       "The time series objects for entry (", key,
                       ") have different x_values. Operation aborted.");
          return std::numeric_limits<double>::infinity();
        }
      }
    }
  }

  double error = 0.0;
  for (auto& pair : ts1.data_) {
    auto& key = pair.first;
    auto& yref = ts1.data_.at(key).y_values;
    auto& ycurrent = ts2.data_.at(key).y_values;
    error += Math::MSE(yref, ycurrent);
  }
  return error;
}

// -----------------------------------------------------------------------------
void TimeSeries::Load(const std::string& full_filepath, TimeSeries** restored) {
  GetPersistentObject(full_filepath.c_str(), "TimeSeries", *restored);
}

// -----------------------------------------------------------------------------
void TimeSeries::Merge(
    TimeSeries* merged, const std::vector<TimeSeries>& time_series,
    const std::function<void(const std::vector<double>&, double*, double*,
                             double*)>& merger) {
  if (!merged) {
    Log::Warning("TimeSeries::Merge",
                 "Parameter 'merged' is a nullptr. Operation aborted.");
    return;
  }

  // check that merged is empty
  if (merged->data_.size() != 0) {
    Log::Warning("TimeSeries::Merge",
                 "Parameter 'merged' is not empty. Operation aborted.");
    return;
  }

  if (time_series.size() == 0) {
    Log::Warning("TimeSeries::Merge",
                 "The given time series vector is empty. Operation aborted.");
    return;
  } else if (time_series.size() == 1) {
    *merged = time_series[0];
    return;
  }

  // verify that all TimeSeries contain the same entries
  auto& ref = time_series[0];
  for (uint64_t i = 1; i < time_series.size(); ++i) {
    auto& current = time_series[i];
    if (ref.data_.size() != current.data_.size()) {
      Log::Warning("TimeSeries::Merge",
                   "The time series objects that should be merged do not "
                   "contain the same entries. Operation aborted.");
      return;
    }
    for (auto& p : ref.data_) {
      if (current.data_.find(p.first) == current.data_.end()) {
        Log::Warning("TimeSeries::Merge",
                     "The time series objects that should be merged do not "
                     "contain the same entries. Operation aborted");
        return;
      }
    }
  }

  for (auto& pair : ref.data_) {
    auto& key = pair.first;
    // check that all time series objects have the same x values
    auto& xref = ref.data_.at(key).x_values;
    for (uint64_t i = 1; i < time_series.size(); ++i) {
      auto& xcurrent = time_series[i].data_.at(key).x_values;
      if (xref.size() != xcurrent.size()) {
        Log::Warning("TimeSeries::Merge", "The time series objects for entry (",
                     key, ") have different x_values. Operation aborted.");
        return;
      } else {
        for (uint64_t j = 0; j < xref.size(); ++j) {
          if (std::abs(xref[j] - xcurrent[j]) > 1e-5) {
            Log::Warning("TimeSeries::Merge",
                         "The time series objects for entry (", key,
                         ") have different x_values. Operation aborted.");
            return;
          }
        }
      }
    }

    // merge
    //  initialize
    merged->data_.emplace(key, Data());
    auto& mdata = merged->data_[key];
    mdata.x_values.resize(xref.size());
    mdata.y_values.resize(xref.size());
    mdata.y_error_low.resize(xref.size());
    mdata.y_error_high.resize(xref.size());

#pragma omp parallel for
    for (uint64_t i = 0; i < xref.size(); ++i) {
      mdata.x_values[i] = xref[i];
      std::vector<double> all_y_values(time_series.size());
      for (uint64_t j = 0; j < time_series.size(); ++j) {
        all_y_values[j] = time_series[j].data_.at(key).y_values[i];
        merger(all_y_values, &mdata.y_values[i], &mdata.y_error_low[i],
               &mdata.y_error_high[i]);
      }
    }
  }
}

// -----------------------------------------------------------------------------
TimeSeries::TimeSeries() = default;

// -----------------------------------------------------------------------------
TimeSeries::TimeSeries(const TimeSeries& other) : data_(other.data_) {}

// -----------------------------------------------------------------------------
TimeSeries::TimeSeries(TimeSeries&& other) noexcept
    : data_(std::move(other.data_)) {}

// -----------------------------------------------------------------------------
TimeSeries& TimeSeries::operator=(TimeSeries&& other) noexcept {
  data_ = std::move(other.data_);
  return *this;
}

// -----------------------------------------------------------------------------
TimeSeries& TimeSeries::operator=(const TimeSeries& other) {
  data_ = other.data_;
  return *this;
}

// -----------------------------------------------------------------------------
void TimeSeries::AddCollector(const std::string& id,
                              double (*ycollector)(Simulation*),
                              double (*xcollector)(Simulation*)) {
  auto it = data_.find(id);
  if (it != data_.end()) {
    Log::Warning("TimeSeries::Add", "TimeSeries with id (", id,
                 ") exists already. Operation aborted.");
  }
  data_.emplace(id, Data(ycollector, xcollector));
}

// -----------------------------------------------------------------------------
void TimeSeries::AddCollector(const std::string& id,
                              Reducer<double>* y_reducer_collector,
                              double (*xcollector)(Simulation*)) {
  auto it = data_.find(id);
  if (it != data_.end()) {
    Log::Warning("TimeSeries::Add", "TimeSeries with id (", id,
                 ") exists already. Operation aborted.");
  }
  data_.emplace(id, Data(y_reducer_collector, xcollector));
}

// -----------------------------------------------------------------------------
void TimeSeries::Update() {
  if (!data_.empty()) {
    auto* sim = Simulation::GetActive();
    auto* scheduler = sim->GetScheduler();
    auto* param = sim->GetParam();
    std::vector<std::pair<Reducer<double>*, const std::string>> reducers;
    for (auto& entry : data_) {
      auto& result_data = entry.second;
      if (result_data.ycollector != nullptr) {
        result_data.y_values.push_back(result_data.ycollector(sim));
      }
      if (result_data.y_reducer_collector != nullptr) {
        result_data.y_reducer_collector->Reset();
        reducers.push_back(
            std::make_pair(result_data.y_reducer_collector, entry.first));
      }
      if (result_data.xcollector == nullptr) {
        result_data.x_values.push_back(scheduler->GetSimulatedSteps() *
                                       param->simulation_time_step);
      } else {
        result_data.x_values.push_back(result_data.xcollector(sim));
      }
    }

    // execute reducers
    auto execute_reducers = L2F([&](Agent* agent) {
      for (auto& el : reducers) {
        (*el.first)(agent);
      }
    });
    sim->GetResourceManager()->ForEachAgentParallel(execute_reducers);
    for (auto& el : reducers) {
      data_[el.second].y_values.push_back(el.first->GetResult());
    }
  }
}

// -----------------------------------------------------------------------------
void TimeSeries::Add(const TimeSeries& ts, const std::string& suffix) {
  if (this == &ts) {
    Log::Warning(
        "TimeSeries::Add",
        "Adding a TimeSeries to itself is not supported. Operation aborted.");
    return;
  }
  for (auto& p : ts.data_) {
    // verify that entry doesn't exist yet
    auto id = Concat(p.first, "-", suffix);
    if (data_.find(id) != data_.end()) {
      Log::Warning("TimeSeries::Add", "TimeSeries with id (", id,
                   ") exists already. Consider changing the suffix parameter "
                   "to make it unique. Operation aborted.");
      return;
    }

    data_.emplace(id, p.second);
  }
}

// -----------------------------------------------------------------------------
void TimeSeries::Add(const std::string& id, const std::vector<double>& x_values,
                     const std::vector<double>& y_values) {
  if (data_.find(id) != data_.end()) {
    Log::Warning("TimeSeries::Add", "TimeSeries with id (", id,
                 ") exists already. Consider changing the suffix parameter to "
                 "make it unique. Operation aborted.");
    return;
  }

  Data data;
  data.x_values = x_values;
  data.y_values = y_values;
  data_.emplace(id, data);
}

// -----------------------------------------------------------------------------
void TimeSeries::Add(const std::string& id, const std::vector<double>& x_values,
                     const std::vector<double>& y_values,
                     const std::vector<double>& y_error_low,
                     const std::vector<double>& y_error_high) {
  if (data_.find(id) != data_.end()) {
    Log::Warning("TimeSeries::Add", "TimeSeries with id (", id,
                 ") exists already. Consider changing the suffix parameter to "
                 "make it unique. Operation aborted.");
    return;
  }

  Data data;
  data.x_values = x_values;
  data.y_values = y_values;
  data.y_error_low = y_error_low;
  data.y_error_high = y_error_high;
  data_.emplace(id, data);
}

// -----------------------------------------------------------------------------
void TimeSeries::Add(const std::string& id, const std::vector<double>& x_values,
                     const std::vector<double>& y_values,
                     const std::vector<double>& y_error) {
  if (data_.find(id) != data_.end()) {
    Log::Warning("TimeSeries::Add", "TimeSeries with id (", id,
                 ") exists already. Consider changing the suffix parameter to "
                 "make it unique. Operation aborted.");
    return;
  }
  Data data;
  data.x_values = x_values;
  data.y_values = y_values;
  data.y_error_low = y_error;
  data.y_error_high = y_error;
  data_.emplace(id, data);
}

// -----------------------------------------------------------------------------
bool TimeSeries::Contains(const std::string& id) const {
  return data_.find(id) != data_.end();
}

// -----------------------------------------------------------------------------
uint64_t TimeSeries::Size() const { return data_.size(); }

// -----------------------------------------------------------------------------
const std::vector<double>& TimeSeries::GetXValues(const std::string& id) const {
  return data_.at(id).x_values;
}

// -----------------------------------------------------------------------------
const std::vector<double>& TimeSeries::GetYValues(const std::string& id) const {
  return data_.at(id).y_values;
}

// -----------------------------------------------------------------------------
const std::vector<double>& TimeSeries::GetYErrorLow(
    const std::string& id) const {
  return data_.at(id).y_error_low;
}

// -----------------------------------------------------------------------------
const std::vector<double>& TimeSeries::GetYErrorHigh(
    const std::string& id) const {
  return data_.at(id).y_error_high;
}

// -----------------------------------------------------------------------------
void TimeSeries::ListEntries() const {
  for (auto& p : data_) {
    std::cout << p.first << std::endl;
  }
}

// -----------------------------------------------------------------------------
void TimeSeries::Save(const std::string& full_filepath) const {
  WritePersistentObject(full_filepath.c_str(), "TimeSeries", *this, "recreate");
}

// -----------------------------------------------------------------------------
void TimeSeries::SaveJson(const std::string& full_filepath) const {
  TBufferJSON::ExportToFile(full_filepath.c_str(), this, Class());
}

}  // namespace experimental
}  // namespace bdm
