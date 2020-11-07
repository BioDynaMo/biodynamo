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
#include "core/simulation.h"

namespace bdm {

ResourceManager::ResourceManager() {
  // Must be called prior any other function call to libnuma
  if (auto ret = numa_available() == -1) {
    Log::Fatal("ResourceManager",
               "Call to numa_available failed with return code: ", ret);
  }
  agents_.resize(numa_num_configured_nodes());

  auto* param = Simulation::GetActive()->GetParam();
  if (param->export_visualization || param->insitu_visualization) {
    type_index_ = new TypeIndex();
  }
}

void ResourceManager::ForEachAgentParallel(
    Functor<void, Agent*, AgentHandle>& function) {
#pragma omp parallel
  {
    auto tid = omp_get_thread_num();
    auto nid = thread_info_->GetNumaNode(tid);
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
    auto& numa_agents = agents_[nid];
    assert(thread_info_->GetNumaNode(tid) == numa_node_of_cpu(sched_getcpu()));

    // use static scheduling for now
    auto correction = numa_agents.size() % threads_in_numa == 0 ? 0 : 1;
    auto chunk = numa_agents.size() / threads_in_numa + correction;
    auto start = thread_info_->GetNumaThreadId(tid) * chunk;
    auto end = std::min(numa_agents.size(), start + chunk);

    for (uint64_t i = start; i < end; ++i) {
      function(numa_agents[i], AgentHandle(nid, i));
    }
  }
}

template <typename TFunctor>
struct ForEachAgentParallelFunctor
    : public Functor<void, Agent*, AgentHandle> {
  TFunctor& functor_;
  ForEachAgentParallelFunctor(TFunctor& f) : functor_(f) {}
  void operator()(Agent* agent, AgentHandle) { functor_(agent); }
};

void ResourceManager::ForEachAgentParallel(
    Functor<void, Agent*>& function) {
  ForEachAgentParallelFunctor<Functor<void, Agent*>> functor(
      function);
  ForEachAgentParallel(functor);
}

void ResourceManager::ForEachAgentParallel(Operation& op) {
  ForEachAgentParallelFunctor<Operation> functor(op);
  ForEachAgentParallel(functor);
}

void ResourceManager::ForEachAgentParallel(
    uint64_t chunk, Functor<void, Agent*, AgentHandle>& function) {
  // adapt chunk size
  auto num_agents = GetNumAgents();
  uint64_t factor = (num_agents / thread_info_->GetMaxThreads()) / chunk;
  chunk = (num_agents / thread_info_->GetMaxThreads()) / (factor + 1);
  chunk = chunk >= 1 ? chunk : 1;

  // use dynamic scheduling
  // Unfortunately openmp's built in functionality can't be used, since
  // threads belong to different numa domains and thus operate on
  // different containers
  auto numa_nodes = thread_info_->GetNumaNodes();
  auto max_threads = omp_get_max_threads();
  std::vector<uint64_t> num_chunks_per_numa(numa_nodes);
  for (int n = 0; n < numa_nodes; n++) {
    auto correction = agents_[n].size() % chunk == 0 ? 0 : 1;
    num_chunks_per_numa[n] = agents_[n].size() / chunk + correction;
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

        auto& numa_agents = agents_[current_nid];
        uint64_t old_count = (*(counters[current_tid]))++;
        while (old_count < max_counters[current_tid]) {
          start = old_count * p_chunk;
          end =
              std::min(static_cast<uint64_t>(numa_agents.size()), start + p_chunk);

          for (uint64_t i = start; i < end; ++i) {
            function(numa_agents[i], AgentHandle(current_nid, i));
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

struct DeleteAgentsFunctor : public Functor<void, Agent*> {
  void operator()(Agent* agent) { delete agent; }
};

struct UpdateUidAgentHandleMapFunctor : public Functor<void, Agent*, AgentHandle> {
  using Map = AgentUidMap<AgentHandle>;
  UpdateUidAgentHandleMapFunctor(Map& rm_uid_ah_map)
      : rm_uid_ah_map_(rm_uid_ah_map) {}

  void operator()(Agent* agent, AgentHandle ah) {
    rm_uid_ah_map_.Insert(agent->GetUid(), ah);
  }

 private:
  Map& rm_uid_ah_map_;
};

struct RearrangeFunctor : public Functor<void, const AgentHandle&> {
  std::vector<std::vector<AgentHandle>>& sorted_agent_handles;
  const std::vector<uint64_t>& agent_per_numa;
  uint64_t cnt = 0;
  uint64_t current_numa = 0;

  RearrangeFunctor(std::vector<std::vector<AgentHandle>>& sorted_agent_handles,
                   const std::vector<uint64_t>& agent_per_numa)
      : sorted_agent_handles(sorted_agent_handles), agent_per_numa(agent_per_numa) {}

  void operator()(const AgentHandle& handle) {
    if (cnt == agent_per_numa[current_numa]) {
      cnt = 0;
      current_numa++;
    }

    sorted_agent_handles[current_numa].push_back(handle);
    cnt++;
  }
};

void ResourceManager::SortAndBalanceNumaNodes() {
  // balance agents per numa node according to the number of
  // threads associated with each numa domain
  auto numa_nodes = thread_info_->GetNumaNodes();
  std::vector<uint64_t> agent_per_numa(numa_nodes);
  uint64_t cummulative = 0;
  auto max_threads = thread_info_->GetMaxThreads();
  for (int n = 1; n < numa_nodes; ++n) {
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(n);
    uint64_t num_agents = GetNumAgents() * threads_in_numa / max_threads;
    agent_per_numa[n] = num_agents;
    cummulative += num_agents;
  }
  agent_per_numa[0] = GetNumAgents() - cummulative;

  // using first touch policy - page will be allocated to the numa domain of
  // the thread that accesses it first.
  // alternative, use numa_alloc_onnode.
  int ret = numa_run_on_node(0);
  if (ret != 0) {
    Log::Fatal("ResourceManager",
               "Run on numa node failed. Return code: ", ret);
  }

  // new data structure for rearranged Agent*
  decltype(agents_) agent_rearranged;
  agent_rearranged.resize(numa_nodes);

  // numa node -> vector of AgentHandles
  std::vector<std::vector<AgentHandle>> sorted_agent_handles;
  sorted_agent_handles.resize(numa_nodes);
#pragma omp parallel for
  for (int n = 0; n < numa_nodes; ++n) {
    if (thread_info_->GetMyNumaNode() == n &&
        thread_info_->GetMyNumaThreadId() == 0) {
      sorted_agent_handles[n].reserve(agent_per_numa[n]);
    }
  }

  auto* env = Simulation::GetActive()->GetEnvironment();
  RearrangeFunctor rearrange(sorted_agent_handles, agent_per_numa);
  env->IterateZOrder(rearrange);

  auto* param = Simulation::GetActive()->GetParam();
  const bool minimize_memory = param->minimize_memory_while_rebalancing;

// create new objects
#pragma omp parallel
  {
    auto tid = thread_info_->GetMyThreadId();
    auto nid = thread_info_->GetNumaNode(tid);

    for (int n = 0; n < numa_nodes; n++) {
      if (nid != n) {
        continue;
      }
      auto& dest = agent_rearranged[n];

      if (thread_info_->GetNumaThreadId(tid) == 0) {
        dest.resize(sorted_agent_handles[n].size());
      }

#pragma omp barrier

      auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
      auto& agent_handles = sorted_agent_handles[n];
      assert(thread_info_->GetNumaNode(tid) ==
             numa_node_of_cpu(sched_getcpu()));

      // use static scheduling
      auto correction = agent_handles.size() % threads_in_numa == 0 ? 0 : 1;
      auto chunk = agent_handles.size() / threads_in_numa + correction;
      auto start = thread_info_->GetNumaThreadId(tid) * chunk;
      auto end = std::min(agent_handles.size(), start + chunk);

      for (uint64_t e = start; e < end; e++) {
        auto& handle = agent_handles[e];
        auto* agent = agents_[handle.GetNumaNode()][handle.GetElementIdx()];
        dest[e] = agent->GetCopy();
        if (type_index_) {
          type_index_->Update(dest[e]);
        }
        if (minimize_memory) {
          delete agent;
        }
      }
    }
  }

  // delete old objects. This approach has a high chance that a thread
  // in the right numa node will delete the object, thus minimizing thread
  // synchronization overheads. The bdm memory allocator does not have this
  // issue.
  if (!minimize_memory) {
    DeleteAgentsFunctor delete_functor;
    ForEachAgentParallel(delete_functor);
  }

  for (int n = 0; n < numa_nodes; n++) {
    agents_[n].swap(agent_rearranged[n]);
  }

  // update uid_ah_map_
  UpdateUidAgentHandleMapFunctor functor(uid_ah_map_);
  ForEachAgentParallel(functor);

  if (Simulation::GetActive()->GetParam()->debug_numa) {
    std::cout << *this << std::endl;
  }
}

}  // namespace bdm
