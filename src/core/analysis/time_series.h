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
#ifndef CORE_ANALYSIS_TIME_SERIES_H_
#define CORE_ANALYSIS_TIME_SERIES_H_

#include <functional>
#include <unordered_map>
#include <vector>
#include "core/util/root.h"

namespace bdm {

class Simulation;

namespace experimental {

class TimeSeries {
 public:
  struct Data {
    double (*collector)(Simulation*) = nullptr;  //!
    std::vector<double> x_values;
    std::vector<double> y_values;
    std::vector<double> y_error_low;
    std::vector<double> y_error_high;
    BDM_CLASS_DEF_NV(Data, 1);
  };

  ///  Restore a saved TimeSeries object.
  ///  Usage example:
  ///
  ///     TimeSeries* ts_restored;
  ///     TimeSeries::Load("path/ts.root", &ts_restored);
  ///
  static void Load(const std::string& full_filepath, TimeSeries** restored);

  static void Merge(
      TimeSeries* merged, const std::vector<TimeSeries>& time_series,
      const std::function<void(const std::vector<double>&, double*, double*,
                               double*)>& merger);

  TimeSeries& operator=(TimeSeries&& other);
  TimeSeries& operator=(const TimeSeries& other);

  // FIXME rename to collect
  void AddCollector(const std::string& id, double (*collector)(Simulation*));

  void Add(const std::string& id, const std::vector<double>& x_values,
           const std::vector<double>& y_values);

  void Add(const TimeSeries& ts, const std::string& suffix);

  void Update();

  bool Contains(const std::string& id) const;
  uint64_t Size() const;
  const std::vector<double>& GetXValues(const std::string& id) const;
  const std::vector<double>& GetYValues(const std::string& id) const;
  const std::vector<double>& GetYErrorLow(const std::string& id) const;
  const std::vector<double>& GetYErrorHigh(const std::string& id) const;

  /// Print all time series entry names to stdout
  void ListEntries() const;

  void Save(const std::string& full_filepath) const;

  void SaveJson(const std::string& full_filepath) const;

 private:
  std::unordered_map<std::string, Data> data_;

  BDM_CLASS_DEF_NV(TimeSeries, 1);
};

// The following custom streamer should be visible to rootcling for dictionary
// generation, but not to the interpreter!
#if (!defined(__CLING__) || defined(__ROOTCLING__)) && defined(USE_DICT)

// The custom streamer is needed because ROOT can't stream function pointers
// by default.
inline void TimeSeries::Data::Streamer(TBuffer& R__b) {
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(TimeSeries::Data::Class(), this);
    Long64_t l;
    R__b.ReadLong64(l);
    this->collector = reinterpret_cast<double (*)(Simulation*)>(l);
  } else {
    R__b.WriteClassBuffer(TimeSeries::Data::Class(), this);
    Long64_t l = reinterpret_cast<Long64_t>(this->collector);
    R__b.WriteLong64(l);
  }
}

#endif  // !defined(__CLING__) || defined(__ROOTCLING__)

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_TIME_SERIES_H_
