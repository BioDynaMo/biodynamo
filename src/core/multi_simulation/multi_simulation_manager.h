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

#ifndef CORE_MULTI_SIMULATION_MULTI_SIMULATION_MANAGER_H_
#define CORE_MULTI_SIMULATION_MULTI_SIMULATION_MANAGER_H_

#ifdef USE_MPI

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "core/analysis/time_series.h"
#include "core/multi_simulation/algorithm/algorithm_registry.h"
#include "core/multi_simulation/dynamic_loop.h"
#include "core/multi_simulation/result_data.h"
#include "core/util/timing_aggregator.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;

namespace bdm {

/// The Master in a Master-Worker design pattern. Maintains the status of all
/// the workers in the MPI runtime.
class MultiSimulationManager {
 public:
  void Log(string s);

  explicit MultiSimulationManager(
      int ws, Param *default_params,
      std::function<void(Param *, TimeSeries *)> simulate);

  void WriteTimingsToFile();

  void IngestData(const std::string &data_file);

  ~MultiSimulationManager();

  // Copy the timing results of the specified worker
  void RecordTiming(int worker, TimingAggregator *agg);

  // Send kill message to all workers
  void KillAllWorkers();

  // Receive timing objects of all workers
  void GetTimingsFromWorkers();

  int Start();

 private:
  friend struct ParticleSwarm;
  // Returns the ID of the first available worker in the list. Returns -1 if
  // there is no available worker.
  int GetFirstAvailableWorker();

  // Changes the status
  void ChangeStatusWorker(int worker, Status s);

  // Executes the specified function for all workers. Starting from index 1,
  // because 0 is the master's ID.
  void ForAllWorkers(const std::function<void(int w)> &lambda);

  vector<Status> availability_;
  int worldsize_;
  TimingAggregator ta_;
  Param *default_params_;
  std::function<void(Param *, TimeSeries *)> simulate_;
  std::vector<TimingAggregator> timings_;
};

/// The Worker class in a Master-Worker design pattern.
class Worker {
 public:
  explicit Worker(int myrank,
                  std::function<void(Param *, TimeSeries *)> simulate);

  template <typename T>
  void Log(T s) {
    Log::Info("MultiSimulationManager", "[W", myrank_, "]:  ", s);
  }

  ~Worker();

  int Start();

 private:
  void IncrementTaskCount();

  int myrank_;
  unsigned int task_count_ = 0;
  std::function<void(Param *, TimeSeries *)> simulate_;
  TimingAggregator ta_;
};

}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_MULTI_SIMULATION_MULTI_SIMULATION_MANAGER_H_
