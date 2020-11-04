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

#include "core/execution_context/in_place_exec_ctxt.h"

#include <algorithm>
#include <mutex>
#include <utility>

#include "core/environment/environment.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/agent/agent.h"

namespace bdm {

InPlaceExecutionContext::ThreadSafeAgentUidMap::ThreadSafeAgentUidMap() {
  map_ = new ThreadSafeAgentUidMap::Map(1000);
  next_ = new ThreadSafeAgentUidMap::Map(static_cast<uint64_t>(map_->size() * 2));
}

InPlaceExecutionContext::ThreadSafeAgentUidMap::~ThreadSafeAgentUidMap() {
  if (map_) {
    delete map_;
  }
  if (next_) {
    delete next_;
  }
  for (auto* map : previous_maps_) {
    delete map;
  }
  previous_maps_.clear();
}

// NB: There is small risk of a race condition here if another thread grows
// the container. In this situation, the element will be inserted in a map
// that will be moved to previous_maps_.
// This race-condition can be mitigated in `ThreadSafeAgentUidMap::operator[]`.
void InPlaceExecutionContext::ThreadSafeAgentUidMap::Insert(
    const AgentUid& uid,
    const typename InPlaceExecutionContext::ThreadSafeAgentUidMap::value_type&
        value) {
  auto index = uid.GetIndex();
  if (map_->size() > index + ThreadInfo::GetInstance()->GetMaxThreads()) {
    map_->Insert(uid, value);
  } else {
    bool resized = false;
    {
      std::lock_guard<Spinlock> guard(lock_);
      // check again
      if (map_->size() <= index + ThreadInfo::GetInstance()->GetMaxThreads()) {
        resized = true;
        if (next_ == nullptr) {
          // another thread is still creating a new map
          // wait until this operation is finished.
          std::lock_guard<Spinlock> guard(next_lock_);
        }
        previous_maps_.emplace_back(map_);
        map_ = next_;
        next_ = nullptr;
      }
    }
    if (resized) {
      std::lock_guard<Spinlock> guard(next_lock_);
      auto new_size =
          std::max(uid.GetIndex(), static_cast<uint32_t>(map_->size() * 2));
      next_ = new ThreadSafeAgentUidMap::Map(new_size);
    }
    // new map might still be too small.
    Insert(uid, value);
  }
}

/// NB: This method fixes any race-condition of `ThreadSafeAgentUidMap::Insert`
/// It might happen that a ThreadSafeAgentUidMap::Insert inserts the element
/// into one of the `previous_maps_`.
/// Therefore, if we don't find the element in `map_`, we iterate through
/// `previous_maps_` to see if we can find it there.
/// This iteration happens only in the unlikely event of a race-condition
/// and is therefore not performance relevant.
const typename InPlaceExecutionContext::ThreadSafeAgentUidMap::value_type&
InPlaceExecutionContext::ThreadSafeAgentUidMap::operator[](const AgentUid& uid) {
  static InPlaceExecutionContext::ThreadSafeAgentUidMap::value_type kDefault;
  auto& pair = (*map_)[uid];
  auto timesteps = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();

  if (pair.second == timesteps && pair.first != nullptr) {
    return pair;
  }

  std::lock_guard<Spinlock> guard(lock_);
  for (auto* m : previous_maps_) {
    if (m->size() <= uid.GetIndex()) {
      continue;
    }
    auto& pair = (*m)[uid];
    if (pair.second == timesteps && pair.first != nullptr) {
      map_->Insert(uid, pair);
      return pair;
    }
  }

  return kDefault;
}

uint64_t InPlaceExecutionContext::ThreadSafeAgentUidMap::Size() const {
  return map_->size();
}
void InPlaceExecutionContext::ThreadSafeAgentUidMap::Resize(uint64_t new_size) {
  map_->resize(new_size);
}

void InPlaceExecutionContext::ThreadSafeAgentUidMap::RemoveOldCopies() {
  for (auto* map : previous_maps_) {
    delete map;
  }
  previous_maps_.clear();
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
  rm->ResizeUidSohMap();
  for (unsigned n = 0; n < new_agent_per_numa.size(); n++) {
    numa_offsets[n] = rm->GrowSoContainer(new_agent_per_numa[n], n);
  }

// add new_agents_ to the ResourceManager in parallel
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = all_exec_ctxts[i];
    int nid = tinfo_->GetNumaNode(i);
    uint64_t offset = thread_offsets[i] + numa_offsets[nid];
    rm->AddNewAgents(nid, offset, ctxt->new_agents_);
    ctxt->new_agents_.clear();
  }

  // remove
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = all_exec_ctxts[i];
    // removed sim objects
    // remove them after adding new ones (maybe one has been removed
    // that was in new_agents_)
    for (auto& uid : ctxt->remove_) {
      rm->Remove(uid);
    }
    ctxt->remove_.clear();
  }

  rm->EndOfIteration();

  // FIXME
  // new_agent_map_.SetOffset(AgentUidGenerator::Get()->GetLastId());
  new_agent_map_->RemoveOldCopies();
  if (rm->GetNumAgents() > new_agent_map_->Size()) {
    new_agent_map_->Resize(rm->GetNumAgents() * 1.5);
  }
}

void InPlaceExecutionContext::Execute(
    Agent* agent, const std::vector<Operation*>& operations) {
  auto* env = Simulation::GetActive()->GetEnvironment();
  auto* param = Simulation::GetActive()->GetParam();

  if (param->thread_safety_mechanism_ ==
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
  } else if (param->thread_safety_mechanism_ ==
             Param::ThreadSafetyMechanism::kAutomatic) {
    auto* nb_mutex_builder = env->GetNeighborMutexBuilder();
    auto* mutex = nb_mutex_builder->GetMutex(agent->GetBoxIdx());
    std::lock_guard<decltype(*mutex)> guard(*mutex);
    neighbor_cache_.clear();
    for (auto* op : operations) {
      (*op)(agent);
    }
  } else if (param->thread_safety_mechanism_ ==
             Param::ThreadSafetyMechanism::kNone) {
    neighbor_cache_.clear();
    for (auto* op : operations) {
      (*op)(agent);
    }
  } else {
    Log::Fatal("InPlaceExecutionContext::Execute",
               "Invalid value for parameter thread_safety_mechanism_: ",
               param->thread_safety_mechanism_);
  }
}

void InPlaceExecutionContext::push_back(Agent* new_so) {  // NOLINT
  new_agents_.push_back(new_so);
  auto timesteps = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  new_agent_map_->Insert(new_so->GetUid(), {new_so, timesteps});
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
    if (param->cache_neighbors_) {
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
    if (param->cache_neighbors_) {
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
