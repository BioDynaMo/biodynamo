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

#include "core/sim_object/sim_object.h"
#include "core/resource_manager.h"
#include "core/grid.h"

namespace bdm {

InPlaceExecutionContext::InPlaceExecutionContext() {}

InPlaceExecutionContext::~InPlaceExecutionContext() {
  for(auto& el : new_sim_objects_) {
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
  for(auto& el : new_sim_objects_) {
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

void InPlaceExecutionContext::Execute(SimObject* so, const std::vector<std::function<void(SimObject*)>>& operations) {
  auto* grid = Simulation::GetActive()->GetGrid();
  auto nb_mutex_builder = grid->GetNeighborMutexBuilder();
  if (nb_mutex_builder != nullptr) {
    auto mutex = nb_mutex_builder->GetMutex(so->GetBoxIdx());
    std::lock_guard<decltype(mutex)> guard(mutex);
    for (auto& op : operations) {
      op(so);
    }
  } else {
    for (auto& op : operations) {
      op(so);
    }
  }
}

void InPlaceExecutionContext::push_back(SimObject* new_so) {
  new_sim_objects_[new_so->GetUid()] = new_so;
}

void InPlaceExecutionContext::ForEachNeighborWithinRadius(const std::function<void(const SimObject*)>& lambda, const SimObject& query,
                                 double squared_radius) {
  auto* grid = Simulation::GetActive()->GetGrid();
  grid->ForEachNeighborWithinRadius(lambda, query, squared_radius);
}

SimObject* InPlaceExecutionContext::GetSimObject(SoUid uid) {
  auto search_it = new_sim_objects_.find(uid);
  if (search_it != new_sim_objects_.end()) {
    return search_it->second;
  } else {
    auto* rm = Simulation::GetActive()->GetResourceManager();
    return rm->GetSimObject(uid);
  }
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

}  // namespace bdm
