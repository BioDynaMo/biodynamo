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
#include "core/environment/environment.h"

namespace bdm {

void ResourceManager::ApplyOnAllElementsParallel(
    Functor<void, SimObject*, SoHandle>& function) {
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
      function(numa_sos[i], SoHandle(nid, i));
    }
  }
}

template <typename TFunctor>
struct ApplyOnAllElementsParallelFunctor
    : public Functor<void, SimObject*, SoHandle> {
  TFunctor& functor_;
  ApplyOnAllElementsParallelFunctor(TFunctor& f)
      : functor_(f) {}
  void operator()(SimObject* so, SoHandle) { functor_(so); }
};

void ResourceManager::ApplyOnAllElementsParallel(
    Functor<void, SimObject*>& function) {
  ApplyOnAllElementsParallelFunctor<Functor<void, SimObject*>> functor(function);
  ApplyOnAllElementsParallel(functor);
}

void ResourceManager::ApplyOnAllElementsParallel(Operation& op) {
  ApplyOnAllElementsParallelFunctor<Operation> functor(op);
  ApplyOnAllElementsParallel(functor);
}

void ResourceManager::ApplyOnAllElementsParallelDynamic(
    uint64_t chunk, Functor<void, SimObject*, SoHandle>& function) {
  // adapt chunk size
  auto num_so = GetNumSimObjects();
  uint64_t factor = (num_so / thread_info_->GetMaxThreads()) / chunk;
  chunk = (num_so / thread_info_->GetMaxThreads()) / (factor + 1);
  chunk = chunk >= 1 ? chunk : 1;

  // use dynamic scheduling
  // Unfortunately openmp's built in functionality can't be used, since
  // threads belong to different numa domains and thus operate on
  // different containers
  auto numa_nodes = thread_info_->GetNumaNodes();
  auto max_threads = omp_get_max_threads();
  std::vector<uint64_t> num_chunks_per_numa(numa_nodes);
  for (int n = 0; n < numa_nodes; n++) {
    auto correction = sim_objects_[n].size() % chunk == 0 ? 0 : 1;
    num_chunks_per_numa[n] = sim_objects_[n].size() / chunk + correction;
  }

  std::vector<std::atomic<uint64_t>*> counters(max_threads, nullptr);
  std::vector<uint64_t> max_counters(max_threads);
  for (int thread_cnt = 0; thread_cnt < max_threads; thread_cnt++) {
    uint64_t current_nid = thread_info_->GetNumaNode(thread_cnt);

    auto correction =
        num_chunks_per_numa[current_nid] %
                    thread_info_->GetThreadsInNumaNode(current_nid) ==
                0
            ? 0
            : 1;
    uint64_t num_chunks_per_thread =
        num_chunks_per_numa[current_nid] /
            thread_info_->GetThreadsInNumaNode(current_nid) +
        correction;
    auto start =
        num_chunks_per_thread * thread_info_->GetNumaThreadId(thread_cnt);
    auto end = std::min(num_chunks_per_numa[current_nid],
                        start + num_chunks_per_thread);

    counters[thread_cnt] = new std::atomic<uint64_t>(start);
    max_counters[thread_cnt] = end;
  }

#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto nid = thread_info_->GetNumaNode(tid);

    // thread private variables (compilation error with
    // firstprivate(chunk, numa_node_) with some openmp versions clause)
    auto p_numa_nodes = thread_info_->GetNumaNodes();
    auto p_max_threads = omp_get_max_threads();
    auto p_chunk = chunk;
    assert(thread_info_->GetNumaNode(tid) == numa_node_of_cpu(sched_getcpu()));

    // dynamic scheduling
    uint64_t start = 0;
    uint64_t end = 0;

    // this loop implements work stealing from other NUMA nodes if there
    // are imbalances. Each thread starts with its NUMA domain. Once, it
    // is finished the thread looks for tasks on other domains
    for (int n = 0; n < p_numa_nodes; n++) {
      int current_nid = (nid + n) % p_numa_nodes;
      for (int thread_cnt = 0; thread_cnt < p_max_threads; thread_cnt++) {
        uint64_t current_tid = (tid + thread_cnt) % p_max_threads;
        if (current_nid != thread_info_->GetNumaNode(current_tid)) {
          continue;
        }

        auto& numa_sos = sim_objects_[current_nid];
        uint64_t old_count = (*(counters[current_tid]))++;
        while (old_count < max_counters[current_tid]) {
          start = old_count * p_chunk;
          end =
              std::min(static_cast<uint64_t>(numa_sos.size()), start + p_chunk);

          for (uint64_t i = start; i < end; ++i) {
            function(numa_sos[i], SoHandle(current_nid, i));
          }

          old_count = (*(counters[current_tid]))++;
        }
      }  // work stealing loop numa_nodes_
    }    // work stealing loop  threads
  }

  for (auto* counter : counters) {
    delete counter;
  }
}

struct DeleteSimObjectsFunctor : public Functor<void, SimObject*> {
  void operator()(SimObject* so) { delete so; }
};

struct UpdateUidSoHMapFunctor : public Functor<void, SimObject*, SoHandle> {
  using Map = SoUidMap<SoHandle>;
  UpdateUidSoHMapFunctor(Map& rm_uid_soh_map)
      : rm_uid_soh_map_(rm_uid_soh_map) {}

  void operator()(SimObject* so, SoHandle soh) {
    rm_uid_soh_map_.Insert(so->GetUid(), soh);
  }

 private:
  Map& rm_uid_soh_map_;
};

struct RearrangeFunctor : public Functor<void, const SoHandle&> {
  std::vector<std::vector<SoHandle>>& sorted_so_handles;
  const std::vector<uint64_t>& so_per_numa;
  uint64_t cnt = 0;
  uint64_t current_numa = 0;

  RearrangeFunctor(std::vector<std::vector<SoHandle>>& sorted_so_handles,
                   const std::vector<uint64_t>& so_per_numa)
      : sorted_so_handles(sorted_so_handles), so_per_numa(so_per_numa) {}

  void operator()(const SoHandle& handle) {
    if (cnt == so_per_numa[current_numa]) {
      cnt = 0;
      current_numa++;
    }

    sorted_so_handles[current_numa].push_back(handle);
    cnt++;
  }
};

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
    Log::Fatal("ResourceManager",
               "Run on numa node failed. Return code: ", ret);
  }

  // new data structure for rearranged SimObject*
  decltype(sim_objects_) so_rearranged;
  so_rearranged.resize(numa_nodes);

  // numa node -> vector of SoHandles
  std::vector<std::vector<SoHandle>> sorted_so_handles;
  sorted_so_handles.resize(numa_nodes);
#pragma omp parallel for
  for (int n = 0; n < numa_nodes; ++n) {
    if (thread_info_->GetMyNumaNode() == n &&
        thread_info_->GetMyNumaThreadId() == 0) {
      sorted_so_handles[n].reserve(so_per_numa[n]);
    }
  }

  auto* env = Simulation::GetActive()->GetEnvironment();
  RearrangeFunctor rearrange(sorted_so_handles, so_per_numa);
  env->IterateZOrder(rearrange);

  auto* param = Simulation::GetActive()->GetParam();
  const bool minimize_memory = param->minimize_memory_while_rebalancing_;

// create new objects
#pragma omp parallel
  {
    auto tid = thread_info_->GetMyThreadId();
    auto nid = thread_info_->GetNumaNode(tid);

    for (int n = 0; n < numa_nodes; n++) {
      if (nid != n) {
        continue;
      }
      auto& dest = so_rearranged[n];

      if (thread_info_->GetNumaThreadId(tid) == 0) {
        dest.resize(sorted_so_handles[n].size());
      }

#pragma omp barrier

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
        if (minimize_memory) {
          delete so;
        }
      }
    }
  }

  // delete old objects. This approach has a high chance that a thread
  // in the right numa node will delete the object, thus minimizing thread
  // synchronization overheads. The bdm memory allocator does not have this
  // issue.
  if (!minimize_memory) {
    DeleteSimObjectsFunctor delete_functor;
    ApplyOnAllElementsParallel(delete_functor);
  }

  for (int n = 0; n < numa_nodes; n++) {
    sim_objects_[n].swap(so_rearranged[n]);
  }

  // update uid_soh_map_
  UpdateUidSoHMapFunctor functor(uid_soh_map_);
  ApplyOnAllElementsParallel(functor);

  if (Simulation::GetActive()->GetParam()->debug_numa_) {
    DebugNuma();
  }
}

void ResourceManager::DebugNuma() const {
  std::cout << "ResourceManager size of sim object containers\n" << std::endl;
  uint64_t cnt = 0;
  for (auto& numa_sos : sim_objects_) {
    std::cout << "  numa node " << cnt++ << " size " << numa_sos.size()
              << std::endl;
  }
}

}  // namespace bdm
