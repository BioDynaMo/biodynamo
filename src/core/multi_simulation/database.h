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

#ifndef CORE_MULTI_SIMULATION_DATABASE_H_
#define CORE_MULTI_SIMULATION_DATABASE_H_

#include "core/analysis/time_series.h"

namespace bdm {
namespace experimental {

using experimental::TimeSeries;

/// A singleton for storing real-life data in the form of a `TimeSeries` object
/// for the purpose of parameter optimization
class Database {
 public:
  TimeSeries data_;

  static Database* GetInstance() {
    static Database kDatabase;
    return &kDatabase;
  }

 private:
  Database() = default;
};

}  // namespace experimental
}  // namespace bdm

#endif  // CORE_MULTI_SIMULATION_DATABASE_H_
