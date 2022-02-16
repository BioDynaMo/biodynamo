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

#include "core/resource_manager.h"
#include <cmath>
#ifndef NDEBUG
#include <set>
#endif  // NDEBUG
#include "core/algorithm.h"
#include "core/container/shared_data.h"
#include "core/environment/environment.h"
#include "core/simulation.h"
#include "core/util/partition.h"
#include "core/util/plot_memory_layout.h"
#include "core/util/timing.h"

namespace bdm {

ResourceManager::ResourceManager() {
  // Must be called prior any other function call to libnuma
  if (auto ret = numa_available() == -1) {
    Log::Fatal("ResourceManager",
               "Call to numa_available failed with return code: ", ret);
  }
  agents_.resize(numa_num_configured_nodes());
  agents_lb_.resize(numa_num_configured_nodes());

  auto* param = Simulation::GetActive()->GetParam();
  if (param->export_visualization || param->insitu_visualization) {
    type_index_ = new TypeIndex();
  }
}

ResourceManager::~ResourceManager() {
  for (auto& el : diffusion_grids_) {
    delete el.second;
  }
  for (auto& numa_agents : agents_) {
    for (auto* agent : numa_agents) {
      delete agent;
    }
  }
  if (type_index_) {
    delete type_index_;
  }
}

void ResourceManager::ForEachAgentParallel(
    Functor<void, Agent*, AgentHandle>& function,
    Functor<bool, Agent*>* filter) {
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
      auto* a = numa_agents[i];
      if (!filter || (filter && (*filter)(a))) {
        function(a, AgentHandle(nid, i));
      }
    }
  }
}

template <typename TFunctor>
struct ForEachAgentParallelFunctor : public Functor<void, Agent*, AgentHandle> {
  TFunctor& functor_;
  explicit ForEachAgentParallelFunctor(TFunctor& f) : functor_(f) {}
  void operator()(Agent* agent, AgentHandle) { functor_(agent); }
};

void ResourceManager::ForEachAgentParallel(Functor<void, Agent*>& function,
                                           Functor<bool, Agent*>* filter) {
  ForEachAgentParallelFunctor<Functor<void, Agent*>> functor(function);
  ForEachAgentParallel(functor, filter);
}

void ResourceManager::ForEachAgentParallel(Operation& op,
                                           Functor<bool, Agent*>* filter) {
  ForEachAgentParallelFunctor<Operation> functor(op);
  ForEachAgentParallel(functor, filter);
}

void ResourceManager::ForEachAgentParallel(
    uint64_t chunk, Functor<void, Agent*, AgentHandle>& function,
    Functor<bool, Agent*>* filter) {
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
          end = std::min(static_cast<uint64_t>(numa_agents.size()),
                         start + p_chunk);

          for (uint64_t i = start; i < end; ++i) {
            auto* a = numa_agents[i];
            if (!filter || (filter && (*filter)(a))) {
              function(a, AgentHandle(current_nid, i));
            }
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

struct LoadBalanceFunctor : public Functor<void, Iterator<AgentHandle>*> {
  bool minimize_memory;
  uint64_t offset;
  uint64_t nid;
  std::vector<std::vector<Agent*>>& agents;
  std::vector<Agent*>& dest;
  AgentUidMap<AgentHandle>& uid_ah_map;
  TypeIndex* type_index;

  LoadBalanceFunctor(bool minimize_memory, uint64_t offset, uint64_t nid,
                     decltype(agents) agents, decltype(dest) dest,
                     decltype(uid_ah_map) uid_ah_map, TypeIndex* type_index)
      : minimize_memory(minimize_memory),
        offset(offset),
        nid(nid),
        agents(agents),
        dest(dest),
        uid_ah_map(uid_ah_map),
        type_index(type_index) {}

  void operator()(Iterator<AgentHandle>* it) {
    while (it->HasNext()) {
      auto handle = it->Next();
      auto* agent = agents[handle.GetNumaNode()][handle.GetElementIdx()];
      auto* copy = agent->NewCopy();
      auto el_idx = offset++;
      dest[el_idx] = copy;
      uid_ah_map.Insert(copy->GetUid(), AgentHandle(nid, el_idx));
      if (type_index) {
        type_index->Update(copy);
      }
      if (minimize_memory) {
        delete agent;
      }
    }
  }
};

void ResourceManager::LoadBalance() {
  // Load balancing destroys the synchronization between the simulation and the
  // environment. We mark the environment aus OutOfSync such that we can update
  // the environment before acessing it again.
  MarkEnvironmentOutOfSync();
  auto* param = Simulation::GetActive()->GetParam();
  if (param->plot_memory_layout) {
    PlotNeighborMemoryHistogram(true);
  }

  // balance agents per numa node according to the number of
  // threads associated with each numa domain
  auto numa_nodes = thread_info_->GetNumaNodes();
  std::vector<uint64_t> agent_per_numa(numa_nodes);
  std::vector<uint64_t> agent_per_numa_cumm(numa_nodes);
  uint64_t cummulative = 0;
  auto max_threads = thread_info_->GetMaxThreads();
  for (int n = 1; n < numa_nodes; ++n) {
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(n);
    uint64_t num_agents = GetNumAgents() * threads_in_numa / max_threads;
    agent_per_numa[n] = num_agents;
    cummulative += num_agents;
  }
  agent_per_numa[0] = GetNumAgents() - cummulative;
  agent_per_numa_cumm[0] = 0;
  for (int n = 1; n < numa_nodes; ++n) {
    agent_per_numa_cumm[n] = agent_per_numa_cumm[n - 1] + agent_per_numa[n - 1];
  }

  // using first touch policy - page will be allocated to the numa domain of
  // the thread that accesses it first.
  // alternative, use numa_alloc_onnode.
  int ret = numa_run_on_node(0);
  if (ret != 0) {
    Log::Fatal("ResourceManager",
               "Run on numa node failed. Return code: ", ret);
  }
  auto* env = Simulation::GetActive()->GetEnvironment();
  auto lbi = env->GetLoadBalanceInfo();

  const bool minimize_memory = param->minimize_memory_while_rebalancing;

// create new agents
#pragma omp parallel
  {
    auto tid = thread_info_->GetMyThreadId();
    auto nid = thread_info_->GetNumaNode(tid);

    auto& dest = agents_lb_[nid];
    if (thread_info_->GetNumaThreadId(tid) == 0) {
      if (dest.capacity() < agent_per_numa[nid]) {
        dest.reserve(agent_per_numa[nid] * 1.5);
      }
      dest.resize(agent_per_numa[nid]);
    }

#pragma omp barrier

    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
    assert(thread_info_->GetNumaNode(tid) == numa_node_of_cpu(sched_getcpu()));

    // use static scheduling
    auto correction = agent_per_numa[nid] % threads_in_numa == 0 ? 0 : 1;
    auto chunk = agent_per_numa[nid] / threads_in_numa + correction;
    auto start =
        thread_info_->GetNumaThreadId(tid) * chunk + agent_per_numa_cumm[nid];
    auto end =
        std::min(agent_per_numa_cumm[nid] + agent_per_numa[nid], start + chunk);

    LoadBalanceFunctor f(minimize_memory, start - agent_per_numa_cumm[nid], nid,
                         agents_, dest, uid_ah_map_, type_index_);
    lbi->CallHandleIteratorConsumer(start, end, f);
  }

  // delete old objects. This approach has a high chance that a thread
  // in the right numa node will delete the object, thus minimizing thread
  // synchronization overheads. The bdm memory allocator does not have this
  // issue.
  if (!minimize_memory) {
    auto delete_functor = L2F([](Agent* agent) { delete agent; });
    ForEachAgentParallel(delete_functor);
  }

  for (int n = 0; n < numa_nodes; n++) {
    agents_[n].swap(agents_lb_[n]);
    if (param->plot_memory_layout) {
      PlotMemoryLayout(agents_[n], n);
      PlotMemoryHistogram(agents_[n], n);
    }
  }
  if (param->plot_memory_layout) {
    PlotNeighborMemoryHistogram();
  }

  if (Simulation::GetActive()->GetParam()->debug_numa) {
    std::cout << *this << std::endl;
  }
}

// -----------------------------------------------------------------------------
void ResourceManager::RemoveAgents(
    const std::vector<std::vector<AgentUid>*>& uids) {
  // initialization
  // cumulative numbers of to be removed agents
  auto numa_nodes = thread_info_->GetNumaNodes();
  std::vector<std::vector<uint64_t>> tbr_cum(numa_nodes);
  for (auto& el : tbr_cum) {
    el.resize(uids.size() + 1);
  }

  std::vector<uint64_t> remove(numa_nodes);
  std::vector<uint64_t> lowest(numa_nodes);
  parallel_remove_.to_right.resize(numa_nodes);
  parallel_remove_.not_to_left.resize(numa_nodes);
  // thread offsets into to_right and not_to_left
  std::vector<SharedData<uint64_t>> start(numa_nodes);
  // number of swaps in each block
  // add one more element to have enough space for exclusive prefix sum
  std::vector<SharedData<uint64_t>> swaps_to_right(numa_nodes);
  std::vector<SharedData<uint64_t>> swaps_to_left(numa_nodes);

#ifndef NDEBUG
  std::set<AgentUid> toberemoved;
#endif  // NDEBUG

  // determine how many agents will be removed in each numa domain
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < uids.size(); ++i) {
    for (auto& uid : *uids[i]) {
      auto ah = uid_ah_map_[uid];
      tbr_cum[ah.GetNumaNode()][i]++;
    }
  }

#pragma omp parallel
  {
    auto nid = thread_info_->GetMyNumaNode();
    auto ntid = thread_info_->GetMyNumaThreadId();
    if (thread_info_->GetMyNumaThreadId() == 0) {
      ExclusivePrefixSum(&tbr_cum[nid], tbr_cum[nid].size() - 1);
      remove[nid] = tbr_cum[nid].back();
      lowest[nid] = agents_[nid].size() - remove[nid];

      if (remove[nid] != 0) {
        if (parallel_remove_.to_right[nid].capacity() < remove[nid]) {
          parallel_remove_.to_right[nid].reserve(remove[nid] * 1.5);
        }
        if (parallel_remove_.not_to_left[nid].capacity() < remove[nid]) {
          parallel_remove_.not_to_left[nid].reserve(remove[nid] * 1.5);
        }
      }
    }
#pragma omp barrier
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
    uint64_t start_init = 0;
    uint64_t end_init = 0;
    Partition(remove[nid], threads_in_numa, ntid, &start_init, &end_init);
    for (uint64_t i = start_init; i < end_init; ++i) {
      parallel_remove_.to_right[nid][i] = std::numeric_limits<uint64_t>::max();
      parallel_remove_.not_to_left[nid][i] = 0;
    }
  }

  // find agents that must be swapped
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < uids.size(); ++i) {
    uint64_t cnt = 0;
    for (auto& uid : *uids[i]) {
      assert(ContainsAgent(uid));
      auto ah = uid_ah_map_[uid];
      auto nid = ah.GetNumaNode();
      auto eidx = ah.GetElementIdx();
#ifndef NDEBUG
#pragma omp critical
      toberemoved.insert(uid);
#endif  // NDEBUG

      if (eidx < lowest[nid]) {
        parallel_remove_.to_right[nid][tbr_cum[nid][i] + cnt] = eidx;
      } else {
        parallel_remove_.not_to_left[nid][eidx - lowest[nid]] = 1;
      }
      cnt++;
    }
  }

  // reorder
#pragma omp parallel
  {
    auto nid = thread_info_->GetMyNumaNode();
    auto ntid = thread_info_->GetMyNumaThreadId();
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);

    if (remove[nid] != 0) {
      if (ntid == 0) {
        start[nid].resize(threads_in_numa);
        swaps_to_left[nid].resize(threads_in_numa + 1);
        swaps_to_right[nid].resize(threads_in_numa + 1);
      }
    }
#pragma omp barrier
    if (remove[nid] != 0) {
      uint64_t end = 0;
      Partition(remove[nid], threads_in_numa, ntid, &start[nid][ntid], &end);

      for (uint64_t i = start[nid][ntid]; i < end; ++i) {
        if (parallel_remove_.to_right[nid][i] !=
            std::numeric_limits<uint64_t>::max()) {
          parallel_remove_
              .to_right[nid][start[nid][ntid] + swaps_to_right[nid][ntid]++] =
              parallel_remove_.to_right[nid][i];
        }
        if (!parallel_remove_.not_to_left[nid][i]) {
          // here the interpretation of not_to_left changes to to_left
          // just to reuse memory
          parallel_remove_
              .not_to_left[nid][start[nid][ntid] + swaps_to_left[nid][ntid]++] =
              i;
        }
      }
    }
#pragma omp barrier
    if (remove[nid] != 0) {
      // calculate exclusive prefix sum for number of swaps in each block
      if (ntid == 0) {
        ExclusivePrefixSum(&swaps_to_right[nid],
                           swaps_to_right[nid].size() - 1);
        ExclusivePrefixSum(&swaps_to_left[nid], swaps_to_left[nid].size() - 1);
      }
    }
#pragma omp barrier
    if (remove[nid] != 0) {
      uint64_t num_swaps = swaps_to_right[nid][threads_in_numa];
      if (num_swaps != 0) {
        // perform swaps
        uint64_t swap_start = 0;
        uint64_t swap_end = 0;
        Partition(num_swaps, threads_in_numa, ntid, &swap_start, &swap_end);

        if (swap_start < swap_end) {
          auto tr_block = BinarySearch(swap_start, swaps_to_right[nid], 0,
                                       swaps_to_right[nid].size() - 1);
          auto tl_block = BinarySearch(swap_start, swaps_to_left[nid], 0,
                                       swaps_to_left[nid].size() - 1);

          auto tr_block_swaps =
              swaps_to_right[nid][tr_block + 1] - swaps_to_right[nid][tr_block];
          auto tl_block_swaps =
              swaps_to_left[nid][tl_block + 1] - swaps_to_left[nid][tl_block];

          // number of elements to discard in the beginning
          auto tr_block_idx = swap_start - swaps_to_right[nid][tr_block];
          auto tl_block_idx = swap_start - swaps_to_left[nid][tl_block];

          for (uint64_t s = swap_start; s < swap_end; ++s) {
            // calculate element indices that should be swapped
            auto tr_idx = start[nid][tr_block] + tr_block_idx;
            auto tl_idx = start[nid][tl_block] + tl_block_idx;
            auto tr_eidx = parallel_remove_.to_right[nid][tr_idx];
            auto tl_eidx =
                parallel_remove_.not_to_left[nid][tl_idx] + lowest[nid];

            // swap
            assert(tl_eidx < agents_[nid].size());
            assert(tr_eidx < agents_[nid].size());
            auto* reordered = agents_[nid][tl_eidx];
#ifndef NDEBUG
            assert(toberemoved.find(agents_[nid][tl_eidx]->GetUid()) ==
                   toberemoved.end());
            assert(toberemoved.find(agents_[nid][tr_eidx]->GetUid()) !=
                   toberemoved.end());
#endif  // NDBUG
            agents_[nid][tl_eidx] = agents_[nid][tr_eidx];
            agents_[nid][tr_eidx] = reordered;
            uid_ah_map_.Insert(reordered->GetUid(), AgentHandle(nid, tr_eidx));

            // find next pair
            if (swap_end - s > 1) {
              // right
              tr_block_idx++;
              if (tr_block_idx >= tr_block_swaps) {
                tr_block_idx = 0;
                tr_block_swaps = 0;
                while (!tr_block_swaps) {
                  tr_block++;
                  tr_block_swaps = swaps_to_right[nid][tr_block + 1] -
                                   swaps_to_right[nid][tr_block];
                }
              }
              // left
              tl_block_idx++;
              if (tl_block_idx >= tl_block_swaps) {
                tl_block_idx = 0;
                tl_block_swaps = 0;
                while (!tl_block_swaps) {
                  tl_block++;
                  tl_block_swaps = swaps_to_left[nid][tl_block + 1] -
                                   swaps_to_left[nid][tl_block];
                }
              }
            }
          }
        }
      }
    }
#pragma omp barrier
    if (remove[nid] != 0) {
      // delete agents
      uint64_t start_del = 0;
      uint64_t end_del = 0;
      Partition(remove[nid], threads_in_numa, ntid, &start_del, &end_del);

      start_del += lowest[nid];
      end_del += lowest[nid];

      for (uint64_t i = start_del; i < end_del; ++i) {
        Agent* agent = agents_[nid][i];
        assert(toberemoved.find(agent->GetUid()) != toberemoved.end());
        uid_ah_map_.Remove(agent->GetUid());
        if (type_index_) {
          // TODO parallelize type_index removal
#pragma omp critical
          type_index_->Remove(agent);
        }
        delete agent;
      }
    }
  }
  // shrink container
  for (uint64_t n = 0; n < agents_.size(); ++n) {
    agents_[n].resize(lowest[n]);
  }
  MarkEnvironmentOutOfSync();
}

// -----------------------------------------------------------------------------
size_t ResourceManager::GetAgentVectorCapacity(int numa_node) {
  return agents_[numa_node].capacity();
}

// -----------------------------------------------------------------------------
void ResourceManager::SwapAgents(std::vector<std::vector<Agent*>>* agents) {
  agents_.swap(*agents);
}

void ResourceManager::MarkEnvironmentOutOfSync() {
  auto* env = Simulation::GetActive()->GetEnvironment();
  env->MarkAsOutOfSync();
}

}  // namespace bdm
