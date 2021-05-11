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

#include <unordered_map>
#include <vector>
#include "core/util/root.h"

namespace bdm {

class Simulation;

namespace experimental {

class TimeSeries {
 public:
  TimeSeries& operator=(TimeSeries&& other);

  void Add(const std::string& id, double (*f)(Simulation*));

  void Update();

  const std::vector<double>& GetXValues(const std::string& id) const;

  const std::vector<double>& GetYValues(const std::string& id) const;

  // TODO void Save() const;
 private:
  struct Data {
    double (*f)(Simulation*);
    std::vector<double> x_values;
    std::vector<double> y_values;
  };
  std::unordered_map<std::string, Data> data_;

  BDM_CLASS_DEF_NV(TimeSeries, 1);
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_ANALYSIS_TIME_SERIES_H_
