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

#include "core/resource_manager.h"
#include "core/grid.h"

namespace bdm {

void ResourceManager::ApplyOnAllElementsParallel(
    const std::function<void(SimObject*)>& function) {
#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto nid = thread_info_->GetNumaNode(tid);
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
    auto& numa_sos = sim_objects_[nid];
    assert(thread_info_->GetNumaNode(tid) == numa_node_of_cpu(sched_getcpu()));

    // use static scheduling for now
    auto correction = numa_sos.size() % threads_in_numa == 0 ? 0 : 1;
    auto chunk = numa_sos.size() / threads_in_numa + correction;
    auto start = thread_info_->GetNumaThreadId(tid) * chunk;
    auto end = std::min(numa_sos.size(), start + chunk);

    for (uint64_t i = start; i < end; ++i) {
      function(numa_sos[i]);
    }
  }
}

void ResourceManager::ApplyOnAllElementsParallelDynamic(
    uint64_t chunk, const std::function<void(SimObject*, SoHandle)>& function) {
  // use dynamic scheduling
  // Unfortunately openmp's built in functionality can't be used, since
  // threads belong to different numa domains and thus operate on
  // different containers
  auto numa_nodes = thread_info_->GetNumaNodes();
  std::vector<std::atomic<uint64_t>*> counters(numa_nodes, nullptr);
  std::vector<uint64_t> max_counters(numa_nodes);
  for (int n = 0; n < numa_nodes; n++) {
    counters[n] = new std::atomic<uint64_t>(0);
    // calculate value max_counters for each numa domain
    auto correction = sim_objects_[n].size() % chunk == 0 ? 0 : 1;
    max_counters[n] = sim_objects_[n].size() / chunk + correction;
  }

#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto nid = thread_info_->GetNumaNode(tid);

    // thread private variables (compilation error with
    // firstprivate(chunk, numa_node_) with some openmp versions clause)
    auto p_numa_nodes = thread_info_->GetNumaNodes();
    auto p_chunk = chunk;
    assert(thread_info_->GetNumaNode(tid) == numa_node_of_cpu(sched_getcpu()));

    // dynamic scheduling
    uint64_t start = 0;
    uint64_t end = 0;

    // this loop implements work stealing from other NUMA nodes if there
    // are imbalances. Each thread starts with its NUMA domain. Once, it
    // is finished the thread looks for tasks on other domains
    for (int n = 0; n < p_numa_nodes; n++) {
      uint64_t current_nid = (nid + n) % p_numa_nodes;

      auto& numa_sos = sim_objects_[current_nid];
      uint64_t old_count = (*(counters[current_nid]))++;
      while (old_count <= max_counters[current_nid]) {
        start = old_count * p_chunk;
        end = std::min(static_cast<uint64_t>(numa_sos.size()), start + p_chunk);

        for (uint64_t i = start; i < end; ++i) {
          function(numa_sos[i], SoHandle(current_nid, i));
        }

        old_count = (*(counters[current_nid]))++;
      }
    }
  }

  for (int n = 0; n < numa_nodes; n++) {
    delete counters[n];
  }
}

void ResourceManager::SortAndBalanceNumaNodes() {
  // balance simulation objects per numa node according to the number of
  // threads associated with each numa domain
  auto numa_nodes = thread_info_->GetNumaNodes();
  std::vector<uint64_t> so_per_numa(numa_nodes);
  uint64_t cummulative = 0;
  auto max_threads = thread_info_->GetMaxThreads();
  for (int n = 1; n < numa_nodes; ++n) {
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(n);
    uint64_t num_so = GetNumSimObjects() * threads_in_numa / max_threads;
    so_per_numa[n] = num_so;
    cummulative += num_so;
  }
  so_per_numa[0] = GetNumSimObjects() - cummulative;

  // using first touch policy - page will be allocated to the numa domain of
  // the thread that accesses it first.
  // alternative, use numa_alloc_onnode.
  int ret = numa_run_on_node(0);
  if (ret != 0) {
    Log::Fatal("ResourceManager", "Run on numa node failed. Return code: ",
               ret);
  }

  // new data structure for rearranged SimObject*
  decltype(sim_objects_) so_rearranged;
  so_rearranged.resize(numa_nodes);

  // numa node -> vector of SoHandles
  std::vector<std::vector<SoHandle>> sorted_so_handles;
  sorted_so_handles.resize(numa_nodes);

  uint64_t cnt = 0;
  uint64_t current_numa = 0;

  auto rearrange = [&](const SoHandle& handle) {
    if (cnt == so_per_numa[current_numa]) {
      cnt = 0;
      current_numa++;
    }

    sorted_so_handles[current_numa].push_back(handle);
    cnt++;
  };

  auto* grid = Simulation::GetActive()->GetGrid();
  grid->IterateZOrder(rearrange);

  for (int n = 0; n < numa_nodes; n++) {
    auto& dest = so_rearranged[n];
    std::atomic<bool> resized(false);
#pragma omp parallel
    {
      auto tid = omp_get_thread_num();
      auto nid = thread_info_->GetNumaNode(tid);
      if (nid == n) {
        auto old = std::atomic_exchange(&resized, true);
        if (!old) {
          dest.resize(sorted_so_handles[n].size());
        }
      }
    }
#pragma omp parallel
    {
      auto tid = omp_get_thread_num();
      auto nid = thread_info_->GetNumaNode(tid);

      if (nid == n) {
        auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
        auto& sohandles = sorted_so_handles[n];
        assert(thread_info_->GetNumaNode(tid) ==
               numa_node_of_cpu(sched_getcpu()));

        // use static scheduling
        auto correction = sohandles.size() % threads_in_numa == 0 ? 0 : 1;
        auto chunk = sohandles.size() / threads_in_numa + correction;
        auto start = thread_info_->GetNumaThreadId(tid) * chunk;
        auto end = std::min(sohandles.size(), start + chunk);

        for (uint64_t e = start; e < end; e++) {
          auto& handle = sohandles[e];
          auto* so = sim_objects_[handle.GetNumaNode()][handle.GetElementIdx()];
          dest[e] = so->GetCopy();
          delete so;
        }
      }
    }
  }

  for (int n = 0; n < numa_nodes; n++) {
    sim_objects_[n].swap(so_rearranged[n]);
  }

  // update uid_soh_map_
  ApplyOnAllElements([this](SimObject* so, SoHandle soh) {
    this->uid_soh_map_[so->GetUid()] = soh;
  });

  // FIXME
  // // TODO(lukas) do we need this? we don't change the scheduling anymore
  // thread_info_->Renew();
}

}  // namespace bdm
