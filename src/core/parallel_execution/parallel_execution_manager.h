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

#ifndef CORE_PARALLEL_EXECUTION_PARALLEL_EXECUTION_MANAGER_H_
#define CORE_PARALLEL_EXECUTION_PARALLEL_EXECUTION_MANAGER_H_

#ifdef USE_MPI

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "mpi.h"

#include "core/parallel_execution/algorithm/algorithm_registry.h"
#include "core/parallel_execution/dynamic_loop.h"
#include "core/parallel_execution/util.h"
#include "core/parallel_execution/xml_parser.h"
#include "core/scheduler.h"
#include "core/util/log.h"
#include "core/util/timing.h"
#include "core/util/timing_aggregator.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;

namespace bdm {

/// The Master in a Master-Worker design pattern. Maintains the status of all
/// the workers in the MPI runtime.
class ParallelExecutionManager {
 public:
  void Log(string s);

  explicit ParallelExecutionManager(int ws, Param *default_params,
                                    std::function<void(Param *)> simulate);

  void WriteTimingsToFile();

  ~ParallelExecutionManager();

  // Copy the timing results of the specified worker
  void RecordTiming(int worker, TimingAggregator *agg);

  // Register all workers' availability
  void WaitForAllWorkers();

  // Send kill message to all workers
  void KillAllWorkers();

  // Receive timing objects of all workers
  void GetTimingsFromWorkers();

  int Start();

 private:
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
  std::function<void(Param *)> simulate_;
  std::vector<TimingAggregator> timings_;
};

/// The Worker class in a Master-Worker design pattern.
class Worker {
 public:
  explicit Worker(int myrank, std::function<void(Param *)> simulate);

  template <typename T>
  void Log(T s) {
    Log::Info("ParallelExecutionManager", "[W", myrank_, "]:  ", s);
  }

  ~Worker();

  int Start();

 private:
  void IncrementTaskCount();

  int myrank_;
  unsigned int task_count_ = 0;
  std::function<void(Param *)> simulate_;
  TimingAggregator ta_;
};

}  // namespace bdm

#endif  // USE_MPI

#endif  // CORE_PARALLEL_EXECUTION_PARALLEL_EXECUTION_MANAGER_H_
