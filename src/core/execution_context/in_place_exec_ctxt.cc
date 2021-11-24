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

#include "core/execution_context/in_place_exec_ctxt.h"

#include <algorithm>
#include <mutex>
#include <utility>

#include "core/agent/agent.h"
#include "core/environment/environment.h"
#include "core/functor.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"

namespace bdm {

InPlaceExecutionContext::ThreadSafeAgentUidMap::ThreadSafeAgentUidMap()
    : batches_(nullptr) {
  Resize(kBatchSize);
}

InPlaceExecutionContext::ThreadSafeAgentUidMap::~ThreadSafeAgentUidMap() {
  DeleteOldCopies();
  auto* non_atomic_batches = batches_.load();
  if (non_atomic_batches != nullptr) {
    for (uint64_t i = 0; i < num_batches_; ++i) {
      delete non_atomic_batches[i];
    }
    delete[] non_atomic_batches;
  }
}

void InPlaceExecutionContext::ThreadSafeAgentUidMap::Insert(
    const AgentUid& uid,
    const typename InPlaceExecutionContext::ThreadSafeAgentUidMap::value_type&
        value) {
  auto index = uid.GetIndex();
  auto bidx = index / kBatchSize;
  if (bidx >= num_batches_) {
    auto new_size =
        std::max(index, static_cast<uint32_t>(num_batches_ * kBatchSize * 1.1));
    Resize(new_size);
  }

  auto el_idx = index % kBatchSize;
  (*batches_.load()[bidx])[el_idx] = value;
}

const typename InPlaceExecutionContext::ThreadSafeAgentUidMap::value_type&
InPlaceExecutionContext::ThreadSafeAgentUidMap::operator[](
    const AgentUid& uid) {
  static InPlaceExecutionContext::ThreadSafeAgentUidMap::value_type kDefault;
  auto index = uid.GetIndex();
  auto bidx = index / kBatchSize;
  if (bidx >= num_batches_) {
    Log::Fatal("ThreadSafeAgentUidMap::operator[]",
               Concat("AgentUid out of range access: AgentUid: ", uid,
                      ", ThreadSafeAgentUidMap max index ",
                      num_batches_ * kBatchSize));
    return kDefault;
  }

  auto el_idx = index % kBatchSize;
  return (*batches_.load()[bidx])[el_idx];
}

uint64_t InPlaceExecutionContext::ThreadSafeAgentUidMap::Size() const {
  return num_batches_ * kBatchSize;
}

void InPlaceExecutionContext::ThreadSafeAgentUidMap::Resize(uint64_t new_size) {
  using Batch = InPlaceExecutionContext::ThreadSafeAgentUidMap::Batch;
  auto new_num_batches = new_size / kBatchSize + 1;
  std::lock_guard<Spinlock> guard(lock_);
  if (new_num_batches >= num_batches_) {
    auto bcopy = new Batch*[new_num_batches];
    auto** non_atomic_batches = batches_.load();
    for (uint64_t i = 0; i < new_num_batches; ++i) {
      if (i < num_batches_) {
        bcopy[i] = non_atomic_batches[i];
      } else {
        bcopy[i] = new Batch();
        bcopy[i]->reserve(kBatchSize);
      }
    }
    batches_.exchange(bcopy);
    old_copies_.push_back(non_atomic_batches);
    num_batches_ = new_num_batches;
  }
}

void InPlaceExecutionContext::ThreadSafeAgentUidMap::DeleteOldCopies() {
  for (auto& entry : old_copies_) {
    if (entry != nullptr) {
      delete[] entry;
    }
  }
  old_copies_.clear();
}

InPlaceExecutionContext::InPlaceExecutionContext(
    const std::shared_ptr<ThreadSafeAgentUidMap>& map)
    : new_agent_map_(map), tinfo_(ThreadInfo::GetInstance()) {
  new_agents_.reserve(1e3);
  cache_neighbors_ = Simulation::GetActive()->GetParam()->cache_neighbors;
}

InPlaceExecutionContext::~InPlaceExecutionContext() {
  for (auto* agent : new_agents_) {
    delete agent;
  }
}

void InPlaceExecutionContext::SetupIterationAll(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {
  // first iteration might have uncommited changes
  AddAgentsToRm(all_exec_ctxts);
  RemoveAgentsFromRm(all_exec_ctxts);
}

void InPlaceExecutionContext::TearDownIterationAll(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {
  AddAgentsToRm(all_exec_ctxts);
  RemoveAgentsFromRm(all_exec_ctxts);

  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->EndOfIteration();
}

void InPlaceExecutionContext::SetupAgentOpsAll(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {}

void InPlaceExecutionContext::TearDownAgentOpsAll(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {}

void InPlaceExecutionContext::Execute(
    Agent* agent, AgentHandle ah, const std::vector<Operation*>& operations) {
  auto* env = Simulation::GetActive()->GetEnvironment();
  auto* param = Simulation::GetActive()->GetParam();

  if (param->thread_safety_mechanism ==
      Param::ThreadSafetyMechanism::kUserSpecified) {
    while (true) {
      critical_region_.clear();
      critical_region_2_.clear();
      locks_.clear();
      agent->CriticalRegion(&critical_region_);
      std::sort(critical_region_.begin(), critical_region_.end());
      for (auto uid : critical_region_) {
        locks_.push_back(GetAgent(uid)->GetLock());
      }
      for (auto* l : locks_) {
        l->lock();
      }
      agent->CriticalRegion(&critical_region_2_);
      std::sort(critical_region_2_.begin(), critical_region_2_.end());
      bool same = true;
      if (critical_region_.size() == critical_region_2_.size()) {
        for (size_t i = 0; i < critical_region_.size(); ++i) {
          if (critical_region_[i] != critical_region_2_[i]) {
            same = false;
            break;
          }
        }
      } else {
        same = false;
      }
      if (same) {
        break;
      }
      for (auto* l : locks_) {
        l->unlock();
      }
    }
    neighbor_cache_.clear();
    cached_squared_search_radius_ = 0;
    for (auto& op : operations) {
      (*op)(agent);
    }
    for (auto* l : locks_) {
      l->unlock();
    }
  } else if (param->thread_safety_mechanism ==
             Param::ThreadSafetyMechanism::kAutomatic) {
    auto* nb_mutex_builder = env->GetNeighborMutexBuilder();
    auto* mutex = nb_mutex_builder->GetMutex(agent->GetBoxIdx());
    std::lock_guard<decltype(*mutex)> guard(*mutex);
    neighbor_cache_.clear();
    cached_squared_search_radius_ = 0;
    for (auto* op : operations) {
      (*op)(agent);
    }
  } else if (param->thread_safety_mechanism ==
             Param::ThreadSafetyMechanism::kNone) {
    neighbor_cache_.clear();
    cached_squared_search_radius_ = 0;
    for (auto* op : operations) {
      (*op)(agent);
    }
  } else {
    Log::Fatal("InPlaceExecutionContext::Execute",
               "Invalid value for parameter thread_safety_mechanism: ",
               param->thread_safety_mechanism);
  }
}

void InPlaceExecutionContext::AddAgent(Agent* new_agent) {
  new_agents_.push_back(new_agent);
  new_agent_map_->Insert(new_agent->GetUid(), new_agent);
}

bool InPlaceExecutionContext::IsNeighborCacheValid(
    double query_squared_radius) {
  if (!cache_neighbors_) {
    return false;
  }

  // A neighbor cache is valid when the cached search radius was greater or
  // equal than the current search radius (i.e. all the agents within the
  // current neighbor cache are at least in the cache).
  // The agents that are between the cached and the current search radius must
  // be filtered out hereafter
  return query_squared_radius <= cached_squared_search_radius_;
}

void InPlaceExecutionContext::ForEachNeighbor(Functor<void, Agent*>& lambda,
                                              const Agent& query,
                                              void* criteria) {
  // forward call to env and populate cache
  auto* env = Simulation::GetActive()->GetEnvironment();
  auto for_each = L2F([&](Agent* agent) { lambda(agent); });
  env->ForEachNeighbor(for_each, query, criteria);
}

void InPlaceExecutionContext::ForEachNeighbor(
    Functor<void, Agent*, double>& lambda, const Agent& query,
    double squared_radius) {
  // use values in cache
  if (IsNeighborCacheValid(squared_radius)) {
    for (auto& pair : neighbor_cache_) {
      if (pair.second < squared_radius) {
        lambda(pair.first, pair.second);
      }
    }
    return;
  }

  auto* param = Simulation::GetActive()->GetParam();
  auto* env = Simulation::GetActive()->GetEnvironment();

  // Store the search radius to check validity of cache in consecutive use of
  // ForEachNeighbor
  cached_squared_search_radius_ = squared_radius;

  // Populate the cache and execute the lambda for each neighbor
  auto for_each = L2F([&](Agent* agent, double squared_distance) {
    if (param->cache_neighbors) {
      neighbor_cache_.push_back(std::make_pair(agent, squared_distance));
    }
    lambda(agent, squared_distance);
  });
  env->ForEachNeighbor(for_each, query, squared_radius);
}

void InPlaceExecutionContext::ForEachNeighbor(
    Functor<void, Agent*, double>& lambda, const Double3& query_position,
    double squared_radius) {
  auto for_each = L2F([&](Agent* agent, double squared_distance) {
    lambda(agent, squared_distance);
  });
  auto* env = Simulation::GetActive()->GetEnvironment();
  env->ForEachNeighbor(for_each, query_position, squared_radius);
}

Agent* InPlaceExecutionContext::GetAgent(const AgentUid& uid) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* agent = rm->GetAgent(uid);
  if (agent != nullptr) {
    return agent;
  }

  return (*new_agent_map_)[uid];
}

const Agent* InPlaceExecutionContext::GetConstAgent(const AgentUid& uid) {
  return GetAgent(uid);
}

void InPlaceExecutionContext::RemoveAgent(const AgentUid& uid) {
  remove_.push_back(uid);
}

void InPlaceExecutionContext::AddAgentsToRm(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {
  // group execution contexts by numa domain
  std::vector<uint64_t> new_agent_per_numa(tinfo_->GetNumaNodes());
  std::vector<uint64_t> thread_offsets(tinfo_->GetMaxThreads());

  for (int tid = 0; tid < tinfo_->GetMaxThreads(); ++tid) {
    auto* ctxt = bdm_static_cast<InPlaceExecutionContext*>(all_exec_ctxts[tid]);
    int nid = tinfo_->GetNumaNode(tid);
    thread_offsets[tid] = new_agent_per_numa[nid];
    new_agent_per_numa[nid] += ctxt->new_agents_.size();
  }

  // reserve enough memory in ResourceManager
  std::vector<uint64_t> numa_offsets(tinfo_->GetNumaNodes());
  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->ResizeAgentUidMap();
  for (unsigned n = 0; n < new_agent_per_numa.size(); n++) {
    numa_offsets[n] = rm->GrowAgentContainer(new_agent_per_numa[n], n);
  }

// add new_agents_ to the ResourceManager in parallel
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = bdm_static_cast<InPlaceExecutionContext*>(all_exec_ctxts[i]);
    int nid = tinfo_->GetNumaNode(i);
    uint64_t offset = thread_offsets[i] + numa_offsets[nid];
    rm->AddAgents(nid, offset, ctxt->new_agents_);
    ctxt->new_agents_.clear();
  }

  new_agent_map_->DeleteOldCopies();
  if (rm->GetNumAgents() > new_agent_map_->Size()) {
    new_agent_map_->Resize(rm->GetNumAgents() * 1.5);
  }
}

void InPlaceExecutionContext::RemoveAgentsFromRm(
    const std::vector<ExecutionContext*>& all_exec_ctxts) {
  std::vector<decltype(remove_)*> all_remove(tinfo_->GetMaxThreads());

  auto num_removals = 0;
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = bdm_static_cast<InPlaceExecutionContext*>(all_exec_ctxts[i]);
    all_remove[i] = &ctxt->remove_;
    num_removals += ctxt->remove_.size();
  }

  if (num_removals != 0) {
    auto* rm = Simulation::GetActive()->GetResourceManager();
    rm->RemoveAgents(all_remove);

    for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
      auto* ctxt = bdm_static_cast<InPlaceExecutionContext*>(all_exec_ctxts[i]);
      ctxt->remove_.clear();
    }
  }
}
// TODO(lukas) Add tests for caching mechanism in ForEachNeighbor*

}  // namespace bdm
