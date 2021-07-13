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
#include "core/analysis/reduce.h"
#include "core/util/root.h"

namespace bdm {

class Simulation;

namespace experimental {

/// This class simplifies the collection of time series data during a
/// simulation. Every entry has an id and data arrays storing x-values,
/// y-values, y-error-low, and y-error-high.
class TimeSeries {
 public:
  struct Data {
    Data();
    /// TODO
    Data(double (*ycollector)(Simulation*), double (*xcollector)(Simulation*));
    /// TODO
    Data(Reducer<double>* y_reducer_collector,
         double (*xcollector)(Simulation*));
    Data(const Data&);
    ~Data();

    Data& operator=(const Data& other) = delete;

    Reducer<double>* y_reducer_collector = nullptr;
    double (*ycollector)(Simulation*) = nullptr;  //!
    double (*xcollector)(Simulation*) = nullptr;  //!
    std::vector<double> x_values;
    std::vector<double> y_values;
    std::vector<double> y_error_low;
    std::vector<double> y_error_high;
    BDM_CLASS_DEF_NV(Data, 1);
  };

  /// Restore a saved TimeSeries object.
  /// Usage example:
  /// \code
  /// TimeSeries* ts_restored;
  /// TimeSeries::Load("path/ts.root", &ts_restored);
  /// \endcode
  static void Load(const std::string& full_filepath, TimeSeries** restored);

  /// This function combines several time series into one.
  /// All time series in parameter `time_series` must have the same entries.
  /// All entries must have the exact same x values.
  /// The parameter `merger` takes a function which describes how each data
  /// point should be combined.
  /// Let's assume the following example: We take the median of each value
  /// and define error_low = mean - minimum, and error_high = maximum - mean.
  /// All time series objects have one entry called "entry-0" with the same
  /// x-values.
  /// \code
  /// std::vector<TimeSeries> tss(3);
  /// tss[0].Add("entry-0", {1, 2}, {2, 5});
  /// tss[1].Add("entry-0", {1, 2}, {4, 8});
  /// tss[2].Add("entry-0", {1, 2}, {1, 13});
  /// TimeSeries merged;
  /// TimeSeries::Merge(
  ///     &merged, tss,
  ///     [](const std::vector<double>& all_y_values, double* y, double* el,
  ///        double* eh) {
  ///       *y = TMath::Median(all_y_values.size(), all_y_values.data());
  ///       *el = *y - *TMath::LocMin(all_y_values.begin(), all_y_values.end());
  ///       *eh = *TMath::LocMax(all_y_values.begin(), all_y_values.end()) - *y;
  ///     });
  /// \endcode
  /// After these operations, `merged` will contain one entry with id "entry-0"
  /// with the following arrays: \n
  /// `x-values:     {1, 2}` \n
  /// `y-values:     {2, 8}` \n
  /// `y-error-low:  {1, 3}` \n
  /// `y-error-high: {2, 5}` \n
  /// Of course any other merger can be used too: e.g. mean + stddev
  /// \see https://root.cern/doc/master/namespaceTMath.html
  static void Merge(
      TimeSeries* merged, const std::vector<TimeSeries>& time_series,
      const std::function<void(const std::vector<double>&, double*, double*,
                               double*)>& merger);

  TimeSeries();
  TimeSeries(const TimeSeries& other);
  TimeSeries(TimeSeries&& other);

  TimeSeries& operator=(TimeSeries&& other);
  TimeSeries& operator=(const TimeSeries& other);

  /// Adds a new collector which is executed at each iteration.
  /// e.g. to track the number of agents in the simulation:
  /// \code
  /// auto* ts = simulation.GetTimeSeries();
  /// auto get_num_agents = [](Simulation* sim) {
  ///   return static_cast<double>(sim->GetResourceManager()->GetNumAgents());
  /// };
  /// ts->AddCollector("num-agents", get_num_agents);
  /// \endcode
  /// The optional x-value collector allows to modify the x-values.
  /// If no x-value collector is given, x-values will correspond to the
  /// simulation time.
  void AddCollector(const std::string& id, double (*ycollector)(Simulation*),
                    double (*xcollector)(Simulation*) = nullptr);

  /// TODO
  void AddCollector(const std::string& id, Reducer<double>* y_reducer_collector,
                    double (*xcollector)(Simulation*) = nullptr);

  /// Add new entry with data that is not collected during a simulation.
  /// This function can for example be used to add experimental data
  /// which can be later plotted together with the simulation results
  /// using a `LineGraph`.
  /// \code
  /// time_series.Add("experimental-data", {0, 1, 2}, {3, 4, 5});
  /// \endcode
  void Add(const std::string& id, const std::vector<double>& x_values,
           const std::vector<double>& y_values);

  void Add(const std::string& id, const std::vector<double>& x_values,
           const std::vector<double>& y_values,
           const std::vector<double>& y_error);

  void Add(const std::string& id, const std::vector<double>& x_values,
           const std::vector<double>& y_values,
           const std::vector<double>& y_error_low,
           const std::vector<double>& y_error_high);

  /// Add the entries of another TimeSeries instance to this one.
  /// Let's assume that `ts` contains the entries:
  /// "entry1" and "entry2" and that suffix is set to "-from-ts".
  /// In this scenario the following entries will be added to this
  /// object: "entry1-from-ts", "entry2-from-ts"
  void Add(const TimeSeries& ts, const std::string& suffix);

  /// Adds a new data point to all time series with a collector.
  void Update();

  /// Returns whether a times series with given id exists in this object.
  bool Contains(const std::string& id) const;
  uint64_t Size() const;
  const std::vector<double>& GetXValues(const std::string& id) const;
  const std::vector<double>& GetYValues(const std::string& id) const;
  const std::vector<double>& GetYErrorLow(const std::string& id) const;
  const std::vector<double>& GetYErrorHigh(const std::string& id) const;

  /// Print all time series entry names to stdout
  void ListEntries() const;

  /// Saves a root file to disk.
  void Save(const std::string& full_filepath) const;

  /// Saves a json representation to disk
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
    this->ycollector = reinterpret_cast<double (*)(Simulation*)>(l);
    R__b.ReadLong64(l);
    this->xcollector = reinterpret_cast<double (*)(Simulation*)>(l);
  } else {
    R__b.WriteClassBuffer(TimeSeries::Data::Class(), this);
    Long64_t l = reinterpret_cast<Long64_t>(this->ycollector);
    R__b.WriteLong64(l);
    l = reinterpret_cast<Long64_t>(this->xcollector);
    R__b.WriteLong64(l);
  }
}

#endif  // !defined(__CLING__) || defined(__ROOTCLING__)

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_TIME_SERIES_H_
