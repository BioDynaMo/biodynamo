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

#include "core/resource_manager.h"
#include <cmath>
#include "core/algorithm.h"
#include "core/container/shared_data.h"
#include "core/environment/environment.h"
#include "core/simulation.h"
#include "core/util/partition.h"
#include <set> // FIXME remove
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
struct ForEachAgentParallelFunctor : public Functor<void, Agent*, AgentHandle> {
  TFunctor& functor_;
  explicit ForEachAgentParallelFunctor(TFunctor& f) : functor_(f) {}
  void operator()(Agent* agent, AgentHandle) { functor_(agent); }
};

void ResourceManager::ForEachAgentParallel(Functor<void, Agent*>& function) {
  ForEachAgentParallelFunctor<Functor<void, Agent*>> functor(function);
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
          end = std::min(static_cast<uint64_t>(numa_agents.size()),
                         start + p_chunk);

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

struct LoadBalanceFunctor : public Functor<void, Iterator<AgentHandle>*> {
  bool minimize_memory;
  uint64_t offset;
  uint64_t offset_in_numa;
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

  auto* param = Simulation::GetActive()->GetParam();
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
    DeleteAgentsFunctor delete_functor;
    ForEachAgentParallel(delete_functor);
  }

  for (int n = 0; n < numa_nodes; n++) {
    agents_[n].swap(agents_lb_[n]);
  }

  if (Simulation::GetActive()->GetParam()->debug_numa) {
    std::cout << *this << std::endl;
  }
}

// -----------------------------------------------------------------------------
void ResourceManager::RemoveAgents(
    const std::vector<std::vector<AgentUid>*>& uids) {
  auto startts = Timing::Timestamp();
  // initialization
  // cumulative numbers of to be removed agents
  auto numa_nodes = thread_info_->GetNumaNodes();
  std::vector<std::vector<uint64_t>> tbr_cum(numa_nodes);
  for (auto& el : tbr_cum) {
    el.resize(uids.size() + 1);
  }

  std::vector<uint64_t> remove(numa_nodes);
  std::vector<uint64_t> lowest(numa_nodes);
  to_right.resize(numa_nodes);
  not_to_left.resize(numa_nodes);
  // thread offsets into to_right and not_to_left
  std::vector<SharedData<uint64_t>> start(numa_nodes);
  // number of swaps in each block
  // add one more element to have enough space for exclusive prefix sum
  std::vector<SharedData<uint64_t>> swaps_to_right(numa_nodes);
  std::vector<SharedData<uint64_t>> swaps_to_left(numa_nodes);

  // std::set<AgentUid> toberemoved;

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
      // calculate exclusive prefix sum for tbr_cum
      auto tmp = tbr_cum[nid][0];
      tbr_cum[nid][0] = 0;
      for (uint64_t i = 1; i < tbr_cum[nid].size(); ++i) {
        remove[nid] += tmp;
        auto result = tbr_cum[nid][i - 1] + tmp;
        tmp = tbr_cum[nid][i];
        tbr_cum[nid][i] = result;
      }
      // for (auto& el : tbr_cum) {
      //   std::cout << "cum " << el << std::endl;
      // }
      lowest[nid] = agents_[nid].size() - remove[nid];
      // std::cout << "lowest " << lowest[nid] << std::endl;

      if (remove[nid] != 0) {
        if (to_right[nid].capacity() < remove[nid]) {
          to_right[nid].reserve(remove[nid] * 1.5);
        }
        // to_right[nid].resize(remove[nid], std::numeric_limits<uint64_t>::max());
        if (not_to_left[nid].capacity() < remove[nid]) {
          not_to_left[nid].reserve(remove[nid] * 1.5);
        }
        // not_to_left[nid].resize(remove[nid]);
      }
      // std::cout << "to_right size " << to_right[nid].size() << std::endl;
      // std::cout << "not_to_left size " << not_to_left[nid].size() <<
      // std::endl;
    }
#pragma omp barrier
    auto threads_in_numa = thread_info_->GetThreadsInNumaNode(nid);
    uint64_t start_init = 0;
    uint64_t end_init = 0;
    Partition(remove[nid], threads_in_numa, ntid, &start_init, &end_init);
    for (uint64_t i = start_init; i < end_init; ++i) {
      to_right[nid][i] = std::numeric_limits<uint64_t>::max();
      not_to_left[nid][i] = 0;
    }
  }

  // std::cout << "Init " << (Timing::Timestamp() - startts) << std::endl; 
 // for(uint64_t nid = 0; nid < numa_nodes; ++nid) {
 // } 
 
  // find agents that must be swapped
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = 0; i < uids.size(); ++i) {
      uint64_t cnt = 0;
      // #pragma omp critical
      for (auto& uid : *uids[i]) {
        assert(ContainsAgent(uid));
        auto ah = uid_ah_map_[uid];
        auto nid = ah.GetNumaNode();
        auto eidx = ah.GetElementIdx();
// #pragma omp critical
//         toberemoved.insert(uid);
        // std::cout << "toberemoved " << uid << std::endl;
        // std::cout << "tid " << i << " uid " << uid << " ah " << ah << " edix"
        // << eidx << std::endl;
        if (eidx < lowest[nid]) {
          to_right[nid][tbr_cum[nid][i] + cnt] = eidx;
          // std::cout << "   to right uid " << uid << " ah "<< ah  << " || i "
          // << i << " tbr_cum[i] " << tbr_cum[i] << " cnt " << cnt <<
          // std::endl;
        } else {
          not_to_left[nid][eidx - lowest[nid]] = 1;
          // std::cout << "   not to left " << ah  << " eidx " << (eidx -
          // lowest)
          // << std::endl;
        }
        cnt++;
      }
  }

  // for (auto* a : agents_[0]) {
  //   std::cout << "here " << a->GetUid() << " " << uid_ah_map_[a->GetUid()]
  //             << std::endl;
  // }
  // for (uint64_t i = 0; i < to_right[0].size(); ++i) {
  //   std::cout << "swap arrays " << to_right[0][i] << " - " <<
  //   not_to_left[0][i]
  //             << std::endl;
  // }

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
// #pragma omp critical
    if (remove[nid] != 0) {
      uint64_t end = 0;
      // #pragma omp critical
      //       std::cout << nid << " " << threads_in_numa << " " << ntid << " "
      //       << remove[nid]<< std::endl;
      Partition(remove[nid], threads_in_numa, ntid, &start[nid][ntid], &end);

      // #pragma omp critical
      for (uint64_t i = start[nid][ntid]; i < end; ++i) {
        // std::cout << ntid << " - " << i << std::endl;
        if (to_right[nid][i] != std::numeric_limits<uint64_t>::max()) {
          to_right[nid][start[nid][ntid] + swaps_to_right[nid][ntid]++] =
              to_right[nid][i];
          // std::cout << tid << " to_right " << start[tid] +
          // swaps_to_right[tid]
          // << ":" << to_right[i] << std::endl;
        }
        if (!not_to_left[nid][i]) {
          // here the interpretation of not_to_left changes to to_left
          // just to reuse memory
          // std::cout << tid << " to_left " << start[tid] + swaps_to_left[tid]
          // <<
          // ":" << i << std::endl;
          not_to_left[nid][start[nid][ntid] + swaps_to_left[nid][ntid]++] = i;
        }
      }
    }
#pragma omp barrier
// #pragma omp critical
    if (remove[nid] != 0) {
      // #pragma omp critical
      //       if (ntid == 0) {
      //       for (uint64_t i = 0; i < to_right[nid].size(); ++i) {
      //         std::cout << " cum swap arrays " << to_right[nid][i] << " - "
      //         << not_to_left[nid][i]
      //                   << std::endl;
      //       }
      //       for (uint64_t i = 0; i < swaps_to_right[nid].size(); ++i) {
      //         std::cout << "thread swap cnts " << swaps_to_right[nid][i] << "
      //         - "
      //                   << swaps_to_left[nid][i] << std::endl;
      //       }
      //       for (uint64_t i = 0; i < start[nid].size(); ++i) {
      //         std::cout << "thread start cnts " << start[nid][i] <<
      //         std::endl;
      //       }
      //       }

      // calculate exclusive prefix sum for number of swaps in each block
      if (ntid == 0) {
        uint64_t tmp_tr = swaps_to_right[nid][0];
        uint64_t tmp_tl = swaps_to_left[nid][0];
        swaps_to_right[nid][0] = 0;
        swaps_to_left[nid][0] = 0;
        for (uint64_t i = 1; i < threads_in_numa + 1; ++i) {
          auto right_res = tmp_tr + swaps_to_right[nid][i - 1];
          auto left_res = tmp_tl + swaps_to_left[nid][i - 1];
          tmp_tr = swaps_to_right[nid][i];
          tmp_tl = swaps_to_left[nid][i];
          swaps_to_right[nid][i] = right_res;
          swaps_to_left[nid][i] = left_res;
        }
      }
    }
#pragma omp barrier
    //   std::cout << "remove[nid] " << remove[nid] << std::endl;
    // std::cout << "FOO" << std::endl;
    //   std::cout << "nid " << nid << std::endl;
    //   std::cout << "threads_in_numa " << threads_in_numa << std::endl;
    //   std::cout << "swaps_to_right[nid].size() " << swaps_to_right[nid].size() << std::endl;
    //   std::cout << "num_swaps " << num_swaps << std::endl;
// #pragma omp critical
    if (remove[nid] != 0) {
      uint64_t num_swaps = swaps_to_right[nid][threads_in_numa];
      if (num_swaps != 0) {
      // for (uint64_t i = 0; i < swaps_to_right.size(); ++i) {
      //   std::cout << "thread swap cnts cum " << swaps_to_right[i] << " - "
      //             << swaps_to_left[i] << std::endl;
      // }
      // std::cout << "num swapts " << num_swaps << std::endl;

      // perform swaps
      uint64_t swap_start = 0;
      uint64_t swap_end = 0;
      Partition(num_swaps, threads_in_numa, ntid, &swap_start, &swap_end);

      if (swap_start < swap_end) {
      auto tr_block = BinarySearch(swap_start, swaps_to_right[nid], 0,
                                   swaps_to_right[nid].size() - 1);
      auto tl_block = BinarySearch(swap_start, swaps_to_left[nid], 0,
                                   swaps_to_left[nid].size() - 1);

// #pragma omp critical
//       {
//       std::cout << "tid " << thread_info_->GetMyThreadId() << std::endl;
//       std::cout << "remove[nid] " << remove[nid] << std::endl;
//       std::cout << "num_swaps " << num_swaps << std::endl;
//       std::cout << "swap_start " << swap_start << std::endl;
//       std::cout << "swap_end " << swap_end << std::endl;
//       for(auto& el : swaps_to_left[nid]) {
//         std::cout << el << std::endl;
//       }
//       std::cout << "L479 " << tl_block << " " << swaps_to_left[nid].size() << std::endl;
//       std::cout << std::endl;
//       }
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
        auto tr_eidx = to_right[nid][tr_idx];
        auto tl_eidx = not_to_left[nid][tl_idx] + lowest[nid];

        // swap
        // #pragma omp critical
        //       {
        //         std::cout << "swap #" << s << " tid " << tid << " " <<
        //         tl_eidx
        //                   << " <-> " << tr_eidx << std::endl;
        //         std::cout << "  li "
        //                   << " right_index " << tr_idx << " right block  " <<
        //                   tr_block
        //                   << " start[tr_block]  " << start[tr_block]
        //                   << " right block index  " << tr_block_idx <<
        //                   std::endl;
        //         std::cout << "  ri "
        //                   << " left_index " << tl_idx << " left block " <<
        //                   tl_block
        //                   << " start[tl_block] " << start[tl_block]
        //                   << " left block index " << tl_block_idx <<
        //                   std::endl;
        //       }
        assert(tl_eidx < agents_[nid].size());
        assert(tr_eidx < agents_[nid].size());
        auto* reordered = agents_[nid][tl_eidx];
// #pragma omp critical
//         {
//         if (toberemoved.find(agents_[nid][tl_eidx]->GetUid()) != toberemoved.end())
//           std::cout << "shouldntbemovedtoleft " << agents_[nid][tl_eidx]->GetUid() << std::endl;
//         if (toberemoved.find(agents_[nid][tr_eidx]->GetUid()) == toberemoved.end())
//           std::cout << "shouldntbemovedtoright " << agents_[nid][tr_eidx]->GetUid() << std::endl;
        // }
        agents_[nid][tl_eidx] = agents_[nid][tr_eidx];
        agents_[nid][tr_eidx] = reordered;
        uid_ah_map_.Insert(reordered->GetUid(), AgentHandle(nid, tr_eidx));
// #pragma omp critical
//         std::cout << "swap tid " << thread_info_->GetMyThreadId() << " nid " << nid << " ntid " << ntid << " " << tr_eidx << " " <<tl_eidx << std::endl;

        // find next pair
        if (swap_end - s > 1) {
          // right
          tr_block_idx++;
          // #pragma omp critical
          //         std::cout << "proceeed " << tid << " lbi " << tr_block_idx
          //         << " lbs "
          //                   << tr_block_swaps << std::endl;
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
          // #pragma omp critical
          //         std::cout << "proceeed " << tid << " rbi " << tl_block_idx
          //         << " rbs "
          //                   << tl_block_swaps << std::endl;
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
// #pragma omp critical
    if (remove[nid] != 0) {
      // for (auto* a : agents_[0]) {
      //   std::cout << "after swap " << a->GetUid() << std::endl;
      // }

      // delete agents
      uint64_t start_del = 0;
      uint64_t end_del = 0;
      Partition(remove[nid], threads_in_numa, ntid, &start_del, &end_del);

      start_del += lowest[nid];
      end_del += lowest[nid];

      for (uint64_t i = start_del; i < end_del; ++i) {
        Agent* agent = agents_[nid][i];
// #pragma omp critical
//         {
//         if (toberemoved.find(agent->GetUid()) == toberemoved.end())
//           std::cout << "shouldntberemoved " << agent->GetUid() << std::endl;
//         }
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
    // for (auto* a : agents_[n]) {
    //   std::cout << "still here " << n << " - " << a->GetUid() << std::endl;
    // }
  }
}

}  // namespace bdm
