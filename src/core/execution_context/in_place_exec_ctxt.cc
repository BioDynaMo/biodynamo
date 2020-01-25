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

#include "core/grid.h"
#include "core/resource_manager.h"
#include "core/scheduler.h"
#include "core/sim_object/sim_object.h"

namespace bdm {

InPlaceExecutionContext::ThreadSafeSoUidMap::ThreadSafeSoUidMap() {
  map_ = new ThreadSafeSoUidMap::Map(
      0);  // Map(1e8);  // FIXME find better initialization value
}

InPlaceExecutionContext::ThreadSafeSoUidMap::~ThreadSafeSoUidMap() {
  if (map_) {
    delete map_;
  }
  for (auto* map : previous_maps_) {
    delete map;
  }
  previous_maps_.clear();
}

// NB: There is small risk of a race condition here if another thread grows
// the container. In this situation, the element will be inserted in a map
// that will be moved to previous_maps_.
// This race-condition can be mitigated in `ThreadSafeSoUidMap::operator[]`.
void InPlaceExecutionContext::ThreadSafeSoUidMap::Insert(
    const SoUid& uid,
    const typename InPlaceExecutionContext::ThreadSafeSoUidMap::value_type&
        value) {
  auto index = uid.GetIndex();
  if (map_->size() > index + ThreadInfo::GetInstance()->GetMaxThreads()) {
    map_->Insert(uid, value);
  } else {
    std::lock_guard<Spinlock> guard(lock_);
    // check again
    if (map_->size() <= index) {
      // map is too small -> grow
      auto* new_map = new Map(*map_);
      new_map->resize(std::max(static_cast<uint64_t>(1000u),
                               static_cast<uint64_t>(map_->size() * 1.5)));
      previous_maps_.emplace_back(map_);
      map_ = new_map;
    }
    map_->Insert(uid, value);
  }
}

/// NB: This method fixes any race-condition of `ThreadSafeSoUidMap::Insert`
/// It might happen that a ThreadSafeSoUidMap::Insert inserts the element
/// into one of the `previous_maps_`.
/// Therefore, if we don't find the element in `map_`, we iterate through
/// `previous_maps_` to see if we can find it there.
/// This iteration happens only in the unlikely event of a race-condition
/// and is therefore not performance relevant.
const typename InPlaceExecutionContext::ThreadSafeSoUidMap::value_type&
    InPlaceExecutionContext::ThreadSafeSoUidMap::operator[](
        const SoUid& uid) {
  static InPlaceExecutionContext::ThreadSafeSoUidMap::value_type kDefault;
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

uint64_t InPlaceExecutionContext::ThreadSafeSoUidMap::Size() const {
  return map_->size();
}
void InPlaceExecutionContext::ThreadSafeSoUidMap::Resize(uint64_t new_size) {
  map_->resize(new_size);
}

void InPlaceExecutionContext::ThreadSafeSoUidMap::RemoveOldCopies() {
  for (auto* map : previous_maps_) {
    delete map;
  }
  previous_maps_.clear();
}

InPlaceExecutionContext::InPlaceExecutionContext(
    const std::shared_ptr<ThreadSafeSoUidMap>& map)
    : new_so_map_(map), tinfo_(ThreadInfo::GetInstance()) {
  new_sim_objects_.reserve(1e3);
}

InPlaceExecutionContext::~InPlaceExecutionContext() {
  for (auto* so : new_sim_objects_) {
    delete so;
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
  std::vector<uint64_t> new_so_per_numa(tinfo_->GetNumaNodes());
  std::vector<uint64_t> thread_offsets(tinfo_->GetMaxThreads());

  for (int tid = 0; tid < tinfo_->GetMaxThreads(); ++tid) {
    auto* ctxt = all_exec_ctxts[tid];
    int nid = tinfo_->GetNumaNode(tid);
    thread_offsets[tid] = new_so_per_numa[nid];
    new_so_per_numa[nid] += ctxt->new_sim_objects_.size();
  }

  // reserve enough memory in ResourceManager
  std::vector<uint64_t> numa_offsets(tinfo_->GetNumaNodes());
  auto* rm = Simulation::GetActive()->GetResourceManager();
  rm->ResizeUidSohMap();
  for (unsigned n = 0; n < new_so_per_numa.size(); n++) {
    numa_offsets[n] = rm->GrowSoContainer(new_so_per_numa[n], n);
  }

// add new_sim_objects_ to the ResourceManager in parallel
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = all_exec_ctxts[i];
    int nid = tinfo_->GetNumaNode(i);
    uint64_t offset = thread_offsets[i] + numa_offsets[nid];
    rm->AddNewSimObjects(nid, offset, ctxt->new_sim_objects_);
    ctxt->new_sim_objects_.clear();
  }

// remove
#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < tinfo_->GetMaxThreads(); i++) {
    auto* ctxt = all_exec_ctxts[i];
    // removed sim objects
    // remove them after adding new ones (maybe one has been removed
    // that was in new_sim_objects_)
    for (auto& uid : ctxt->remove_) {
      rm->Remove(uid);
    }
    ctxt->remove_.clear();
  }

  rm->EndOfIteration();

  // FIXME
  // new_so_map_.SetOffset(SoUidGenerator::Get()->GetLastId());
  new_so_map_->RemoveOldCopies();
  if (rm->GetNumSimObjects() > new_so_map_->Size()) {
    new_so_map_->Resize(rm->GetNumSimObjects() * 1.5);
  }
}

void InPlaceExecutionContext::Execute(
    SimObject* so, const std::vector<Operation>& operations) {
  auto* grid = Simulation::GetActive()->GetGrid();
  // auto nb_mutex_builder = grid->GetNeighborMutexBuilder();
  // if (nb_mutex_builder != nullptr) {
  if (true) {
    // auto mutex = nb_mutex_builder->GetMutex(so->GetBoxIdx());
    // std::lock_guard<decltype(mutex)> guard(mutex);
    std::set<Spinlock*> locks;
    locks.insert(so->GetLock());
    so->CriticalRegion(&locks);
    for(auto* l : locks) {
      l->lock();
    }
    neighbor_cache_.clear();
    for (auto& op : operations) {
      op(so);
    }
    for(auto* l : locks) {
      l->unlock();
    }
  } else {
    neighbor_cache_.clear();
    for (auto& op : operations) {
      op(so);
    }
  }
}

void InPlaceExecutionContext::push_back(SimObject* new_so) {  // NOLINT
  new_sim_objects_.push_back(new_so);
  auto timesteps = Simulation::GetActive()->GetScheduler()->GetSimulatedSteps();
  new_so_map_->Insert(new_so->GetUid(), {new_so, timesteps});
}

void InPlaceExecutionContext::ForEachNeighbor(
    const std::function<void(const SimObject*)>& lambda,
    const SimObject& query) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      lambda(pair.first);
    }
    return;
  }

  auto* grid = Simulation::GetActive()->GetGrid();
  grid->ForEachNeighbor(lambda, query);
}

void InPlaceExecutionContext::ForEachNeighbor(
    const std::function<void(const SimObject*, double)>& lambda,
    const SimObject& query) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      lambda(pair.first, pair.second);
    }
    return;
  }

  // forward call to grid and populate cache
  auto* grid = Simulation::GetActive()->GetGrid();
  auto* param = Simulation::GetActive()->GetParam();
  auto for_each = [&, this](const SimObject* so, double squared_distance) {
    if (param->cache_neighbors_) {
      this->neighbor_cache_.push_back(make_pair(so, squared_distance));
    }
    lambda(so, squared_distance);
  };
  grid->ForEachNeighbor(for_each, query);
}

void InPlaceExecutionContext::ForEachNeighborWithinRadius(
    const std::function<void(const SimObject*)>& lambda, const SimObject& query,
    double squared_radius) {
  // use values in cache
  if (neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      if (pair.second < squared_radius) {
        lambda(pair.first);
      }
    }
    return;
  }

  // forward call to grid and populate cache
  auto* grid = Simulation::GetActive()->GetGrid();
  auto* param = Simulation::GetActive()->GetParam();
  auto for_each = [&, this](const SimObject* so, double squared_distance) {
    if (param->cache_neighbors_) {
      this->neighbor_cache_.push_back(make_pair(so, squared_distance));
    }
    if (squared_distance < squared_radius) {
      lambda(so);
    }
  };
  grid->ForEachNeighbor(for_each, query);
}

SimObject* InPlaceExecutionContext::GetSimObject(const SoUid& uid) {
  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  auto* so = rm->GetSimObject(uid);
  if (so != nullptr) {
    return so;
  }

  // returns nullptr if the object is not found
  return (*new_so_map_)[uid].first;
}

const SimObject* InPlaceExecutionContext::GetConstSimObject(const SoUid& uid) {
  return GetSimObject(uid);
}

void InPlaceExecutionContext::RemoveFromSimulation(const SoUid& uid) {
  remove_.push_back(uid);
}

void InPlaceExecutionContext::DisableNeighborGuard() {
  Simulation::GetActive()->GetGrid()->DisableNeighborMutexes();
}

// TODO(lukas) Add tests for caching mechanism in ForEachNeighbor*

}  // namespace bdm
