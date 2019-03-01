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

#include "core/grid.h"
#include "core/resource_manager.h"
#include "core/sim_object/sim_object.h"

namespace bdm {

InPlaceExecutionContext::InPlaceExecutionContext() {}

InPlaceExecutionContext::~InPlaceExecutionContext() {
  for (auto& el : new_sim_objects_) {
    delete el.second;
  }
}

void InPlaceExecutionContext::SetupIteration() {
  // first iteration might have uncommited changes
  TearDownIteration();
}

void InPlaceExecutionContext::TearDownIteration() {
  // new sim objects
  auto* rm = Simulation::GetActive()->GetResourceManager();
  for (auto& el : new_sim_objects_) {
    rm->push_back(el.second);
  }
  new_sim_objects_.clear();

  // removed sim objects
  // remove them after adding new ones (maybe one has been removed
  // that was in new_sim_objects_)
  for (auto& uid : remove_) {
    rm->Remove(uid);
  }
  remove_.clear();
}

void InPlaceExecutionContext::Execute(
    SimObject* so,
    const std::vector<std::function<void(SimObject*)>>& operations) {
  auto* grid = Simulation::GetActive()->GetGrid();
  auto nb_mutex_builder = grid->GetNeighborMutexBuilder();
  if (nb_mutex_builder != nullptr) {
    auto mutex = nb_mutex_builder->GetMutex(so->GetBoxIdx());
    std::lock_guard<decltype(mutex)> guard(mutex);
    neighbor_cache_.clear();
    for (auto& op : operations) {
      op(so);
    }
  } else {
    neighbor_cache_.clear();
    for (auto& op : operations) {
      op(so);
    }
  }
}

void InPlaceExecutionContext::push_back(SimObject* new_so) {
  while (mutex_.test_and_set(std::memory_order_acquire)) {
  }
  new_sim_objects_[new_so->GetUid()] = new_so;
  mutex_.clear(std::memory_order_release);
}

void InPlaceExecutionContext::ForEachNeighbor(
    const std::function<void(const SimObject*)>& lambda, const SimObject& query) {

  // use values in cache
  if(neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      lambda(pair.first);
    }
    return;
  }

  auto* grid = Simulation::GetActive()->GetGrid();
  grid->ForEachNeighbor(lambda, query);
}

void InPlaceExecutionContext::ForEachNeighbor(
    const std::function<void(const SimObject*,double)>& lambda, const SimObject& query) {

  // use values in cache
  if(neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      lambda(pair.first, pair.second);
    }
    return;
  }

  // forward call to grid and populate cache
  auto* grid = Simulation::GetActive()->GetGrid();
  auto for_each = [&,this](const SimObject* so, double squared_distance) {
    this->neighbor_cache_.push_back(make_pair(so, squared_distance));
    lambda(so, squared_distance);
  };
  grid->ForEachNeighbor(for_each, query);
}

void InPlaceExecutionContext::ForEachNeighborWithinRadius(
    const std::function<void(const SimObject*)>& lambda, const SimObject& query,
    double squared_radius) {

  // use values in cache
  if(neighbor_cache_.size() != 0) {
    for (auto& pair : neighbor_cache_) {
      if (pair.second < squared_radius) {
        lambda(pair.first);
      }
    }
    return;
  }

  // forward call to grid and populate cache
  auto* grid = Simulation::GetActive()->GetGrid();
  auto for_each = [&,this](const SimObject* so, double squared_distance) {
    this->neighbor_cache_.push_back(make_pair(so, squared_distance));
    if (squared_distance < squared_radius) {
      lambda(so);
    }
  };
  grid->ForEachNeighbor(for_each, query);
}

SimObject* InPlaceExecutionContext::GetSimObject(SoUid uid) {
  auto* so = GetCachedSimObject(uid, false);
  if (so != nullptr) { return so; }

  auto* sim = Simulation::GetActive();
  auto* rm = sim->GetResourceManager();
  so = rm->GetSimObject(uid);
  if (so != nullptr) { return so; }

  // sim object must be cached in another InPlaceExecutionContext
  for(auto* ctxt : sim->GetAllExecCtxts()) {
    so = ctxt->GetCachedSimObject(uid);
    if (so != nullptr) { return so; }
  }
  return nullptr;
}

const SimObject* InPlaceExecutionContext::GetConstSimObject(SoUid uid) {
  return GetSimObject(uid);
}

void InPlaceExecutionContext::RemoveFromSimulation(SoUid uid) {
  remove_.push_back(uid);
}

void InPlaceExecutionContext::DisableNeighborGuard() {
  Simulation::GetActive()->GetGrid()->DisableNeighborMutexes();
}

SimObject* InPlaceExecutionContext::GetCachedSimObject(SoUid uid, bool protect) {
  // returning a non const SimObject is not a problem for race conditions.
  // it has to be inside the central or surrounding boxes which are protected
  // if `DisableNeighborGuard` has not been called.
  // while (!protect && mutex_.test_and_set(std::memory_order_acquire)) {
  if(protect) {
    while (mutex_.test_and_set(std::memory_order_acquire)) {}
  }
  SimObject* ret_val = nullptr;
  auto search_it = new_sim_objects_.find(uid);
  if (search_it != new_sim_objects_.end()) {
    ret_val = search_it->second;
  }
  if (protect) {
    mutex_.clear(std::memory_order_release);
  }
  return ret_val;
}

// TODO(lukas) Add tests for caching mechanism in ForEachNeighbor*

}  // namespace bdm
