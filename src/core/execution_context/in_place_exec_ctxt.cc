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

#include "core/execution_context/in_place_exec_ctxt.h"

#include <algorithm>
#include <mutex>
#include <utility>

#include "core/agent/agent.h"
#include "core/environment/environment.h"
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
InPlaceExecutionContext::ThreadSafeAgentUidMap::operator[](const AgentUid& uid) {
  static InPlaceExecutionContext::ThreadSafeAgentUidMap::value_type kDefault;
  auto index = uid.GetIndex();
  auto bidx = index / kBatchSize;
  if (bidx >= num_batches_) {
    Log::Fatal(
        "ThreadSafeAgentUidMap::operator[]",
        Concat("AgentUid out of range access: AgentUid: ", uid,
               ", ThreadSafeAgentUidMap max index ", num_batches_ * kBatchSize));
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
}

InPlaceExecutionContext::~InPlaceExecutionContext() {
  for (auto* agent : new_agents_) {
    delete agent;
  }
}

void InPlaceExecutionContext::SetupIterationAll(
    const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const {
  // first iteration might have uncommited changes
  TearDownIterationAll(all_exec_ctxts);
}

void InPlaceExecutionContext::TearDownIterationAll(
    const std::vector<InPlaceExecutionContext*>& all_exec_ctxts) const {
  // group execution contexts by numa domain
  std::vector<uint64_t> new_agent_per_numa(tinfo_->GetNumaNodes());
  std::vector<uint64_t> thread_offsets(tinfo_->GetMaxThreads());

  for (int tid = 0; tid < tinfo_->GetMaxThreads(); ++tid) {
    auto* ctxt = all_exec_ctxts[tid];
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
    auto* ctxt = all_exec_ctxts[i];
    int nid = tinfo_->GetNumaNode(i);
    uint64_t offset = thread_offsets[i] + numa_offsets[nid];
    rm->AddAgents(nid, offset, ctxt->new_agents_);
    ctxt->new_agents_.clear();
  }

  // remove
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = all_exec_ctxts[i];
    // removed agents
    // remove them after adding new ones (maybe one has been removed
    // that was in new_agents_)
    for (auto& uid : ctxt->remove_) {
      rm->RemoveAgent(uid);
    }
    ctxt->remove_.clear();
  }

  rm->EndOfIteration();

  new_agent_map_->DeleteOldCopies();
  if (rm->GetNumAgents() > new_agent_map_->Size()) {
    new_agent_map_->Resize(rm->GetNumAgents() * 1.5);
  }
}

void InPlaceExecutionContext::Execute(
    Agent* agent, const std::vector<Operation*>& operations) {
  auto* env = Simulation::GetActive()->GetEnvironment();
  auto* param = Simulation::GetActive()->GetParam();

  if (param->thread_safety_mechanism ==
      Param::ThreadSafetyMechanism::kUserSpecified) {
    agent->CriticalRegion(&locks);
    std::sort(locks.begin(), locks.end());
    for (auto* l : locks) {
      l->lock();
    }
    neighbor_cache_.clear();
    for (auto& op : operations) {
      (*op)(agent);
    }
    for (auto* l : locks) {
      l->unlock();
    }
    locks.clear();
  } else if (param->thread_safety_mechanism ==
             Param::ThreadSafetyMechanism::kAutomatic) {
    auto* nb_mutex_builder = env->GetNeighborMutexBuilder();
    auto* mutex = nb_mutex_builder->GetMutex(agent->GetBoxIdx());
    std::lock_guard<decltype(*mutex)> guard(*mutex);
    neighbor_cache_.clear();
    for (auto* op : operations) {
      (*op)(agent);
    }
  } else if (param->thread_safety_mechanism ==
             Param::ThreadSafetyMechanism::kNone) {
    neighbor_cache_.clear();
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
  auto timesteps = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  new_agent_map_->Insert(new_agent->GetUid(), {new_agent, timesteps});
}

struct ForEachNeighborFunctor : public Functor<void, const Agent*, double> {
  const Param* param = Simulation::GetActive()->GetParam();
  Functor<void, const Agent*, double>& function_;
  std::vector<std::pair<const Agent*, double>>& neighbor_cache_;

  ForEachNeighborFunctor(
      Functor<void, const Agent*, double>& function,
      std::vector<std::pair<const Agent*, double>>& neigbor_cache)
      : function_(function), neighbor_cache_(neigbor_cache) {}

  void operator()(const Agent* agent, double squared_distance) override {
    if (param->cache_neighbors) {
      neighbor_cache_.push_back(std::make_pair(agent, squared_distance));
    }
    function_(agent, squared_distance);
  }
};

void InPlaceExecutionContext::ForEachNeighbor(
    Functor<void, const Agent*, double>& lambda, const Agent& query) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      lambda(pair.first, pair.second);
    }
    return;
  }

  // forward call to env and populate cache
  auto* env = Simulation::GetActive()->GetEnvironment();
  ForEachNeighborFunctor for_each(lambda, neighbor_cache_);
  env->ForEachNeighbor(for_each, query);
}

struct ForEachNeighborWithinRadiusFunctor
    : public Functor<void, const Agent*, double> {
  const Param* param = Simulation::GetActive()->GetParam();
  Functor<void, const Agent*, double>& function_;
  std::vector<std::pair<const Agent*, double>>& neighbor_cache_;
  double squared_radius_ = 0;

  ForEachNeighborWithinRadiusFunctor(
      Functor<void, const Agent*, double>& function,
      std::vector<std::pair<const Agent*, double>>& neigbor_cache,
      double squared_radius)
      : function_(function),
        neighbor_cache_(neigbor_cache),
        squared_radius_(squared_radius) {}

  void operator()(const Agent* agent, double squared_distance) override {
    if (param->cache_neighbors) {
      neighbor_cache_.push_back(std::make_pair(agent, squared_distance));
    }
    if (squared_distance < squared_radius_) {
      function_(agent, 0);
    }
  }
};

void InPlaceExecutionContext::ForEachNeighborWithinRadius(
    Functor<void, const Agent*, double>& lambda, const Agent& query,
    double squared_radius) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      if (pair.second < squared_radius) {
        lambda(pair.first, 0);
      }
    }
    return;
  }

  // forward call to env and populate cache
  auto* env = Simulation::GetActive()->GetEnvironment();

  ForEachNeighborWithinRadiusFunctor for_each(lambda, neighbor_cache_,
                                              squared_radius);
  env->ForEachNeighbor(for_each, query);
}

Agent* InPlaceExecutionContext::GetAgent(const AgentUid& uid) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* agent = rm->GetAgent(uid);
  if (agent != nullptr) {
    return agent;
  }

  // returns nullptr if the object is not found
  return (*new_agent_map_)[uid].first;
}

const Agent* InPlaceExecutionContext::GetConstAgent(const AgentUid& uid) {
  return GetAgent(uid);
}

void InPlaceExecutionContext::RemoveFromSimulation(const AgentUid& uid) {
  remove_.push_back(uid);
}

// TODO(lukas) Add tests for caching mechanism in ForEachNeighbor*

}  // namespace bdm
