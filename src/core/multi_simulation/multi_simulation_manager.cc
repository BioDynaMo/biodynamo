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

#ifdef USE_MPI

#include <thread>

#include "mpi.h"

#include "core/multi_simulation/multi_simulation_manager.h"
#include "core/multi_simulation/optimization_param.h"
#include "core/scheduler.h"
#include "core/util/timing.h"

using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::vector;

namespace bdm {

/// The Master in a Master-Worker design pattern. Maintains the status of all
/// the workers in the MPI runtime.
void MultiSimulationManager::Log(string s) {
  Log::Info("MultiSimulationManager", "[M]:   ", s);
}

MultiSimulationManager::MultiSimulationManager(
    int ws, Param *default_params,
    std::function<void(Param *, TimeSeries *)> simulate)
    : worldsize_(ws), default_params_(default_params), simulate_(simulate) {
  Log("Started Master process");
  availability_.resize(ws);
  timings_.resize(ws);
}

void MultiSimulationManager::WriteTimingsToFile() {
  std::ofstream myfile;
  myfile.open("timing_results.csv");
  int worker = 0;
  myfile << "worker_id,simulation_runtime,mpi_runtime" << std::endl;
  for (auto &t : timings_) {
    myfile << worker << ",";
    myfile << t["SIMULATE"] << ",";
    myfile << t["MPI_CALL"] << std::endl;
    worker++;
  }
  myfile.close();
  Log("Timing results of all workers have been written to "
      "timing_results.csv.");
}

MultiSimulationManager::~MultiSimulationManager() {
  Log("Completed all tasks");
}

void MultiSimulationManager::IngestData(const std::string &data_file) {}

// Copy the timing results of the specified worker
void MultiSimulationManager::RecordTiming(int worker, TimingAggregator *agg) {
  timings_[worker] = *agg;
}

// Send kill message to all workers
void MultiSimulationManager::KillAllWorkers() {
  ForAllWorkers([&](int worker) {
    {
      Timing t_mpi("MPI_CALL", &ta_);
      MPI_Send(nullptr, 0, MPI_INT, worker, Tag::kKill, MPI_COMM_WORLD);
    }
  });
}

// Receive timing objects of all workers
void MultiSimulationManager::GetTimingsFromWorkers() {
  ForAllWorkers([&](int worker) {
    int size;
    MPI_Status status;
    {
      Timing t_mpi("MPI_CALL", &ta_);
      MPI_Recv(&size, 1, MPI_INT, MPI_ANY_SOURCE, Tag::kKill, MPI_COMM_WORLD,
               &status);
    }
    TimingAggregator *agg = MPI_Recv_Obj_ROOT<TimingAggregator>(
        size, status.MPI_SOURCE, Tag::kKill);
    RecordTiming(status.MPI_SOURCE, agg);
  });
}

int MultiSimulationManager::Start() {
  {
    Timing t_tot("TOTAL", &ta_);

    // Wait for all workers to reach this barrier
    MPI_Barrier(MPI_COMM_WORLD);

    // Change status of all workers to 'available' after barrier has been
    // reached
    ForAllWorkers(
        [&](int worker) { ChangeStatusWorker(worker, Status::kAvail); });

    auto dispatch_experiment = [&](Param *final_params, TimeSeries *result) {
      // If there is only one MPI process, the master performs the simulation
      if (worldsize_ == 1) {
        simulate_(final_params, result);
      } else {  // Otherwise we dispatch the work to the worker(s)
        auto worker = GetFirstAvailableWorker();

        // If there is no available worker, wait for one to finish
        while (worker == -1) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          worker = GetFirstAvailableWorker();
        }

        // Send parameters to worker
        {
          Timing t_mpi("MPI_CALL", &ta_);
          MPI_Send_Obj_ROOT(final_params, worker, Tag::kTask);
        }

        // Wait for results
        MPI_Status status;
        {
          int size;
          MPI_Recv(&size, 1, MPI_INT, worker, Tag::kResult, MPI_COMM_WORLD,
                   &status);
          string msg =
              "Receiving results from worker " + to_string(status.MPI_SOURCE);
          Log(msg);
          Timing t_mpi("MPI_CALL", &ta_);
          TimeSeries *tmp_result =
              MPI_Recv_Obj_ROOT<TimeSeries>(size, worker, Tag::kResult);
          msg = "Successfully received results from worker " +
                to_string(status.MPI_SOURCE);
          Log(msg);
          *result = *tmp_result;
          delete tmp_result;
        }

        ChangeStatusWorker(status.MPI_SOURCE, Status::kAvail);
      }
    };

    // From default_params read out the OptimizationParam section to
    // determine the algorithm type: e.g. ParameterSweep, Differential
    // Evolution, Particle Swarm Optimization
    OptimizationParam *opt_params = default_params_->Get<OptimizationParam>();
    auto algorithm = CreateOptimizationAlgorithm(opt_params);

    if (algorithm) {
      (*algorithm)(dispatch_experiment, default_params_);
    } else {
      dispatch_experiment(default_params_, new TimeSeries());
    }

    KillAllWorkers();
    GetTimingsFromWorkers();
  }

  // Record master's timing
  RecordTiming(kMaster, &ta_);

  // Write all timing info to file
  WriteTimingsToFile();

  return 0;
}

// Returns the ID of the first available worker in the list. Returns -1 if
// there is no available worker.
int MultiSimulationManager::GetFirstAvailableWorker() {
  int ret = -1;
#pragma omp critical
  {
    auto it = std::find(begin(availability_), end(availability_), true);
    if (it != end(availability_)) {
      ret = std::distance(begin(availability_), it);
      ChangeStatusWorker(ret, Status::kBusy);
    }
  }
  return ret;
}

// Changes the status of a worker
void MultiSimulationManager::ChangeStatusWorker(int worker, Status s) {
  std::stringstream msg;
  msg << "Changing status of [W" << worker << "] to " << s;
  Log(msg.str());
  availability_[worker] = s;
}

// Executes the specified function for all workers. Starting from index 1,
// because 0 is the master's ID.
void MultiSimulationManager::ForAllWorkers(
    const std::function<void(int w)> &lambda) {
  for (int i = 1; i < worldsize_; i++) {
    lambda(i);
  }
}

/// The Worker class in a Master-Worker design pattern.
Worker::Worker(int myrank, std::function<void(Param *, TimeSeries *)> simulate)
    : myrank_(myrank), simulate_(simulate) {
  Log("Started");
}

Worker::~Worker() {
  string msg = "Stopped (Completed " + to_string(task_count_) + " tasks)";
  Log(msg);
}

int Worker::Start() {
  Timing tot("TOTAL", &ta_);

  // Wait for all MPI processes to reach this barrier
  MPI_Barrier(MPI_COMM_WORLD);

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
        Param *params = nullptr;
        {
          Timing t("MPI_CALL", &ta_);
          params = MPI_Recv_Obj_ROOT<Param>(size, kMaster, Tag::kTask);
        }
        // std::stringstream msg;
        // msg << "Now working on parameters: " << *params;
        // Log(msg.str());
        TimeSeries result;
        {
          Timing sim("SIMULATE", &ta_);
          simulate_(params, &result);
        }
        IncrementTaskCount();
        {
          Timing t("MPI_CALL", &ta_);
          Log("Sending back results");
          MPI_Send_Obj_ROOT(&result, kMaster, Tag::kResult);
        }
        break;
      }
      case Tag::kKill:
        // Send back the timing results to the master for writing to file
        MPI_Send_Obj_ROOT<TimingAggregator>(&ta_, kMaster, Tag::kKill);
        return 0;
      default:
        // TODO(ahmad): Should we stop or do something else?
        Log("Received unknown message tag. Stopping...");
        return 1;
    }
  }
}

void Worker::IncrementTaskCount() { task_count_++; }

}  // namespace bdm

#endif  // USE_MPI
