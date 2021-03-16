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
void ResourceManager::RemoveAgents(const std::vector<std::vector<AgentUid>*>& uids) {
  // TODO split into different numa nodes
  // cumulative numbers of to be removed agents
  std::vector<uint64_t> tbr_cum(uids.size() + 1);  
 
  uint64_t remove = 0;
  for (uint64_t i = 1; i < tbr_cum.size(); ++i) {
    remove += uids[i - 1]->size();
    tbr_cum[i] = tbr_cum[i - 1] + uids[i - 1]->size();
  } 
  for(auto& el : tbr_cum) { std::cout << "cum " << el << std::endl; }
  uint64_t lowest = agents_[0].size() - remove;
  std::cout << "lowest " << lowest << std::endl;

  if (lowest == agents_[0].size()) { return ; }
 
  // FIXME change to ParallelResizeVector
  std::vector<uint64_t> to_left(remove, std::numeric_limits<uint64_t>::max());
  std::vector<uint64_t> not_to_right(to_left.size());
  
  std::cout << "to_left size " << to_left.size() << std::endl;
  std::cout << "not_to_right size " << not_to_right.size() << std::endl;
  // find agents that must be swapped
#pragma omp parallel for schedule(static, 1)
  for(uint64_t i = 0; i < uids.size(); ++i) {
    uint64_t cnt = 0;
// #pragma omp critical
    for(auto& uid : *uids[i]) {
      auto ah = uid_ah_map_[uid];
      auto eidx = ah.GetElementIdx();
      // std::cout << "tid " << i << " uid " << uid << " ah " << ah << " edix" << eidx << std::endl;
      if (eidx < lowest) {
        to_left[tbr_cum[i] + cnt] = eidx;
        // std::cout << "   to left uid " << uid << " ah "<< ah  << " || i " << i << " tbr_cum[i] " << tbr_cum[i] << " cnt " << cnt << std::endl;
      } else {
        not_to_right[eidx - lowest] = 1;
        // std::cout << "   not to right " << ah  << " eidx " << (eidx - lowest) << std::endl;
      } 
      cnt++;
    } 
  } 

  for (auto* a : agents_[0]) {
    std::cout << "here " << a->GetUid() << " " << uid_ah_map_[a->GetUid()] << std::endl;
  }
  for (uint64_t i = 0; i < to_left.size(); ++i) {
    std::cout << "swap arrays " << to_left[i] << " - " << not_to_right[i] << std::endl;     
  }

  SharedData<uint64_t> start(thread_info_->GetMaxThreads());
  // add one more element to have enough space for exclusive prefix sum 
  SharedData<uint64_t> swap_cnt_to_left(thread_info_->GetMaxThreads() + 1);
  SharedData<uint64_t> swap_cnt_to_right(thread_info_->GetMaxThreads() + 1);

#pragma omp parallel 
  {
    auto tid = thread_info_->GetMyThreadId(); 
    auto max_threads = thread_info_->GetMaxThreads(); 
    // use static scheduling for now
    uint64_t end = 0;
    Partition(remove, max_threads, tid, &start[tid], &end);

// #pragma omp critical
    for (uint64_t i = start[tid]; i < end; ++i) {
      std::cout << tid << " - " << i << std::endl;
      if (to_left[i] != std::numeric_limits<uint64_t>::max()) {
        to_left[start[tid] + swap_cnt_to_left[tid]++] = to_left[i];
        // std::cout << tid << " to_left " << start[tid] + swap_cnt_to_left[tid] << ":" << to_left[i] << std::endl;
      } 
      if (!not_to_right[i]) {
        // here the interpretation of not_to_right changes to to_right
        // just to reuse memory
      // std::cout << tid << " to_right " << start[tid] + swap_cnt_to_right[tid] << ":" << i << std::endl;
        not_to_right[start[tid] + swap_cnt_to_right[tid]++] = i;
      } 
    }

  }
  
  for (uint64_t i = 0; i < to_left.size(); ++i) {
    std::cout << " cum swap arrays " << to_left[i] << " - " << not_to_right[i] << std::endl;     
  }
  for (uint64_t i = 0; i < swap_cnt_to_left.size(); ++i) {
    std::cout << "thread swap cnts " << swap_cnt_to_left[i] << " - " << swap_cnt_to_right[i] << std::endl;     
  }
  for (uint64_t i = 0; i < start.size(); ++i) {
    std::cout << "thread start cnts " << start[i] << std::endl;     
  }

  // calculate exclusive prefix sum for number of swaps in each batch
  uint64_t tmp_tl = swap_cnt_to_left[0];
  uint64_t tmp_tr = swap_cnt_to_right[0];
  swap_cnt_to_left[0] = 0;  
  swap_cnt_to_right[0] = 0;  
  for(uint64_t i = 1; i < thread_info_->GetMaxThreads() + 1; ++i) {
    auto left_res = tmp_tl + swap_cnt_to_left[i - 1];
    auto right_res = tmp_tr + swap_cnt_to_right[i - 1];
    tmp_tl =  swap_cnt_to_left[i];
    tmp_tr =  swap_cnt_to_right[i];
    swap_cnt_to_left[i] = left_res;
    swap_cnt_to_right[i] = right_res;
  }
  uint64_t num_swaps = swap_cnt_to_left[thread_info_->GetMaxThreads()];
  
  for (uint64_t i = 0; i < swap_cnt_to_left.size(); ++i) {
    std::cout << "thread swap cnts cum " << swap_cnt_to_left[i] << " - " << swap_cnt_to_right[i] << std::endl;     
  }
  std::cout << "num swapts " << num_swaps << std::endl;
  
#pragma omp parallel
  { 
    auto tid = thread_info_->GetMyThreadId(); 
    auto max_threads = thread_info_->GetMaxThreads(); 
    uint64_t swap_start = 0; 
    uint64_t swap_end = 0; 
    Partition(num_swaps, max_threads, tid, &swap_start, &swap_end);
    
    auto left_batch = BinarySearch(swap_start, swap_cnt_to_left, 0, swap_cnt_to_left.size() - 1);
    auto right_batch = BinarySearch(swap_start, swap_cnt_to_right, 0, swap_cnt_to_right.size() - 1);

    auto left_batch_swaps = swap_cnt_to_left[left_batch + 1] - swap_cnt_to_left[left_batch];  
    auto right_batch_swaps = swap_cnt_to_right[right_batch + 1] - swap_cnt_to_right[right_batch];  

    // not 0 but the number of elements to discard in the beginning
    auto left_batch_idx = swap_start - swap_cnt_to_left[left_batch];
    auto right_batch_idx = swap_start - swap_cnt_to_right[right_batch];

    for(uint64_t s = swap_start; s < swap_end; ++s) {
      // calculate element indices that should be swapped
      auto left_idx = start[left_batch] + left_batch_idx;
      auto right_idx = start[right_batch] + right_batch_idx;
      auto left_eidx = to_left[left_idx];
      auto right_eidx = not_to_right[right_idx] + lowest;
      
      // swap
#pragma omp critical
      {
      std::cout << "swap #" << s << " tid " << tid << " " << right_eidx << " <-> " << left_eidx << std::endl;
      std::cout << "  li " << " left_index " << left_idx  << " left batch  " << left_batch << " start[left_batch]  " << start[left_batch] << " left batch index  " << left_batch_idx << std::endl;
      std::cout << "  ri " << " right_index " << right_idx << " right batch " << right_batch << " start[right_batch] " << start[right_batch] << " right batch index " << right_batch_idx << std::endl;
      }
      auto* reordered = agents_[0][right_eidx];
      agents_[0][right_eidx] = agents_[0][left_eidx];
      agents_[0][left_eidx] = reordered;
      uid_ah_map_.Insert(reordered->GetUid(), AgentHandle(0, left_eidx));

      // find next pair
      if (swap_end - s > 1) {
        // left
        left_batch_idx++; 
#pragma omp critical
        std::cout << "proceeed " << tid << " lbi " << left_batch_idx << " lbs " << left_batch_swaps << std::endl;
        if (left_batch_idx >= left_batch_swaps) {
          left_batch_idx = 0;
          left_batch_swaps = 0;
          while(!left_batch_swaps) {
            left_batch++;
            left_batch_swaps = swap_cnt_to_left[left_batch + 1] - swap_cnt_to_left[left_batch];  
          }
        } 
        // right
        right_batch_idx++; 
#pragma omp critical
        std::cout << "proceeed " << tid << " rbi " << right_batch_idx << " rbs " << right_batch_swaps << std::endl;
        if (right_batch_idx >= right_batch_swaps) {
          right_batch_idx = 0;
          right_batch_swaps = 0;
          while(!right_batch_swaps) {
            right_batch++;
            right_batch_swaps = swap_cnt_to_right[right_batch + 1] - swap_cnt_to_right[right_batch];  
          }
        } 
      } 
    }
  }

  for (auto* a : agents_[0]) {
    std::cout << "after swap " << a->GetUid() << std::endl;
  }

  // delete agents
#pragma omp parallel for schedule(static, 1)
  for (uint64_t i = lowest; i < agents_[0].size(); ++i) {
      Agent* agent = agents_[0][i];
      uid_ah_map_.Remove(agent->GetUid());
      if (type_index_) {
        // TODO parallelize type_index removal
        #pragma omp critical
        type_index_->Remove(agent); 
      }
      delete agent;
  }

  // shrink container
  agents_[0].resize(lowest);
  for (auto* a : agents_[0]) {
    std::cout << "still here " << a->GetUid() << std::endl;
  }
}

}  // namespace bdm
