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

#include <algorithm>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "mpi.h"

#include "core/parallel_execution/dynamic_loop.h"
#include "core/parallel_execution/util.h"
#include "core/parallel_execution/xml_parser.h"
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
  void Log(string s) { Log::Info("ParallelExecutionManager", "[M]:   ", s); }

  explicit ParallelExecutionManager(int ws, string xml_file)
      : worldsize_(ws), xml_file_(xml_file) {
    Log("Started Master process");
    availability_.resize(ws);
  }

  ~ParallelExecutionManager() { Log("Completed all tasks"); }

  int Start() {
    XMLParser xp(xml_file_);
    vector<Range> ranges = xp.GetContainer<Range>("range");
    vector<Set> sets = xp.GetContainer<Set>("set");
    // TODO: if there are no sets and ranges, we should run one simulation
    // with the fixed parameters
    std::stringstream ss;
    ss << "Found " << ranges.size() << " range values and " << sets.size();
    ss << " set values";
    Log(ss.str());
    Log("Waiting for workers to register themselves...");
    // Register all workers' availability
    ForAllWorkers([&](int worker) {
      MPI_Recv(nullptr, 0, MPI_INT, worker, Tag::kReady, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      ChangeStatusWorker(worker, Status::kAvail);
    });
    Log("All workers ready\n");

    auto dispatch_params = [&](const std::vector<int> &slots) {
      auto worker = GetFirstAvailableWorker();
      if (worker == -1) {
        MPI_Status status;
        MPI_Recv(nullptr, 0, MPI_INT, MPI_ANY_SOURCE, Tag::kReady,
                 MPI_COMM_WORLD, &status);
        ChangeStatusWorker(status.MPI_SOURCE, Status::kAvail);
      }

      worker = GetFirstAvailableWorker();
      XMLParams params;
      ParamGenerator(&params, slots, ranges, sets);
      std::stringstream msg;
      msg << "Sending parameters to [W" << worker << "]: " << params;
      Log(msg.str());
      MPI_Send_Obj_ROOT(&params, worker, Tag::kTask);
      ChangeStatusWorker(worker, Status::kBusy);
    };
    auto containers = MergeContainers(&ranges, &sets);
    // CHeck if there are any sets or range value types
    if (ranges.size() + sets.size() > 0) {
      DynamicNestedLoop(containers, dispatch_params);
    } else {  // If not, we just dispatch the (what should be scalar) params
      dispatch_params({});
    }

    ForAllWorkers([&](int worker) {
      MPI_Send(nullptr, 0, MPI_INT, worker, Tag::kKill, MPI_COMM_WORLD);
    });

    return 0;
  }

 private:
  // Returns the ID of the first available worker in the list. Returns -1 if
  // there is no available worker.
  int GetFirstAvailableWorker() {
    auto it = std::find(begin(availability_), end(availability_), true);
    if (it == end(availability_)) {
      return -1;
    }
    return std::distance(begin(availability_), it);
  }

  // Changes the status
  void ChangeStatusWorker(int worker, Status s) {
    std::stringstream msg;
    msg << "Changing status of [W" << worker << "] to " << s;
    Log(msg.str());
    availability_[worker] = s;
  }

  // Executes the specified function for all workers. Starting from index 1,
  // because 0 is the master's ID.
  void ForAllWorkers(const std::function<void(int w)> &lambda) {
    for (int i = 1; i < worldsize_; i++) {
      lambda(i);
    }
  }

  vector<Status> availability_;
  int worldsize_;
  std::string xml_file_;
};

/// The Worker class in a Master-Worker design pattern.
class Worker {
 public:
  explicit Worker(int myrank, std::function<int(XMLParams *)> simulate)
      : myrank_(myrank), simulate_(simulate) {
    Log("Started");
  }

  template <typename T>
  void Log(T s) {
    Log::Info("ParallelExecutionManager", "[W", myrank_, "]:  ", s);
  }

  ~Worker() {
    string msg = "Stopped (Completed " + to_string(task_count_) + " tasks)";
    Log(msg);
    Log(ta_);
  }

  int Start() {
    Timing tot("TOTAL", &ta_);
    // Inform Master that I'm ready for work
    {
      Timing t("MPI_CALL", &ta_);
      MPI_Send(nullptr, 0, MPI_INT, kMaster, Tag::kReady, MPI_COMM_WORLD);
    }

    while (true) {
      MPI_Status status;
      int size;
      // Receive the command type. If the command is a task, we use the `size`
      // argument as the size of the object we're about to receive
      {
        Timing t("MPI_CALL", &ta_);
        MPI_Recv(&size, 1, MPI_INT, kMaster, MPI_ANY_TAG, MPI_COMM_WORLD,
                 &status);
      }

      // The tag tells us what kind of message we received from Master
      switch (status.MPI_TAG) {
        case Tag::kTask: {
          XMLParams *params = nullptr;
          {
            Timing t("MPI_CALL", &ta_);
            params = MPI_Recv_Obj_ROOT<XMLParams>(size, Tag::kTask, status);
          }
          std::stringstream msg;
          msg << "Now working on parameters: " << *params;
          Log(msg.str());
          {
            Timing sim("Simulating", &ta_);
            simulate_(params);
          }
          IncrementTaskCount();
          {
            Timing t("MPI_CALL", &ta_);
            MPI_Send(nullptr, 0, MPI_INT, kMaster, Tag::kReady, MPI_COMM_WORLD);
          }
          break;
        }
        case Tag::kKill:
          return 0;
        default:
          // TODO(ahmad): Should we stop or do something else?
          Log("Received unknown message tag. Stopping...");
          return 1;
      }
    }
  }

 private:
  void IncrementTaskCount() { task_count_++; }

  int myrank_;
  unsigned int task_count_ = 0;
  std::function<int(XMLParams *)> simulate_;
  TimingAggregator ta_;
};

}  // namespace bdm

#endif  // CORE_PARALLEL_EXECUTION_PARALLEL_EXECUTION_MANAGER_H_
