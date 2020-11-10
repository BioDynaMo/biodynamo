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

#ifndef CORE_RESOURCE_MANAGER_H_
#define CORE_RESOURCE_MANAGER_H_

#include <omp.h>
#include <sched.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#if defined(USE_OPENCL) && !defined(__ROOTCLING__)
#ifdef __APPLE__
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_PROGRAM_CONSTRUCTION_FROM_ARRAY_COMPATIBILITY
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION 120
#include "cl2.hpp"
#else
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl2.hpp>
#endif
#endif

#include "core/agent/agent.h"
#include "core/agent/agent_handle.h"
#include "core/agent/agent_uid.h"
#include "core/agent/agent_uid_generator.h"
#include "core/container/agent_uid_map.h"
#include "core/diffusion_grid.h"
#include "core/operation/operation.h"
#include "core/simulation.h"
#include "core/type_index.h"
#include "core/util/numa.h"
#include "core/util/root.h"
#include "core/util/thread_info.h"
#include "core/util/type.h"

namespace bdm {

/// ResourceManager stores agents and diffusion grids and provides
/// methods to add, remove, and access them. Agents are uniquely identified
/// by their AgentUid, and AgentHandle. An AgentHandle might change during the
/// simulation.
class ResourceManager {
 public:
  explicit ResourceManager(TRootIOCtor* r) {}

  ResourceManager();

  virtual ~ResourceManager() {
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

  ResourceManager& operator=(ResourceManager&& other) {
    if (agents_.size() != other.agents_.size()) {
      Log::Fatal(
          "Restored ResourceManager has different number of NUMA nodes.");
    }
    for (auto& el : diffusion_grids_) {
      delete el.second;
    }
    for (auto& numa_agents : agents_) {
      for (auto* agent : numa_agents) {
        delete agent;
      }
    }
    agents_ = std::move(other.agents_);
    diffusion_grids_ = std::move(other.diffusion_grids_);

    RebuildAgentUidMap();
    // restore type_index_
    if (type_index_) {
      for (auto& numa_agents : agents_) {
        for (auto* agent : numa_agents) {
          type_index_->Add(agent);
        }
      }
    }
    return *this;
  }

  void RebuildAgentUidMap() {
    // rebuild uid_ah_map_
    uid_ah_map_.clear();
    auto* agent_uid_generator = Simulation::GetActive()->GetAgentUidGenerator();
    uid_ah_map_.resize(agent_uid_generator->GetHighestIndex() + 1);
    for (unsigned n = 0; n < agents_.size(); ++n) {
      for (unsigned i = 0; i < agents_[n].size(); ++i) {
        auto* agent = agents_[n][i];
        this->uid_ah_map_.Insert(agent->GetUid(), AgentHandle(n, i));
      }
    }
  }

  Agent* GetAgent(const AgentUid& uid) {
    if (!uid_ah_map_.Contains(uid)) {
      return nullptr;
    }
    auto& ah = uid_ah_map_[uid];
    return agents_[ah.GetNumaNode()][ah.GetElementIdx()];
  }

  Agent* GetAgent(AgentHandle ah) {
    return agents_[ah.GetNumaNode()][ah.GetElementIdx()];
  }

  AgentHandle GetAgentHandle(const AgentUid& uid) { return uid_ah_map_[uid]; }

  void AddDiffusionGrid(DiffusionGrid* dgrid) {
    uint64_t substance_id = dgrid->GetSubstanceId();
    auto search = diffusion_grids_.find(substance_id);
    if (search != diffusion_grids_.end()) {
      Log::Fatal("ResourceManager::AddDiffusionGrid",
                 "You tried to add a diffusion grid with an already existing "
                 "substance id. Please choose a different substance id.");
    } else {
      diffusion_grids_[substance_id] = dgrid;
    }
  }

  void RemoveDiffusionGrid(size_t substance_id) {
    auto search = diffusion_grids_.find(substance_id);
    if (search != diffusion_grids_.end()) {
      delete search->second;
      diffusion_grids_.erase(search);
    } else {
      Log::Fatal("ResourceManager::AddDiffusionGrid",
                 "You tried to remove a diffusion grid that does not exist.");
    }
  }

  /// Return the diffusion grid which holds the substance of specified id
  DiffusionGrid* GetDiffusionGrid(size_t substance_id) const {
    assert(substance_id < diffusion_grids_.size() &&
           "You tried to access a diffusion grid that does not exist!");
    return diffusion_grids_.at(substance_id);
  }

  /// Return the diffusion grid which holds the substance of specified name
  /// Caution: using this function in a tight loop will result in a slow
  /// simulation. Use `GetDiffusionGrid(size_t)` in those cases.
  DiffusionGrid* GetDiffusionGrid(std::string substance_name) const {
    for (auto& el : diffusion_grids_) {
      auto& dg = el.second;
      if (dg->GetSubstanceName() == substance_name) {
        return dg;
      }
    }
    assert(false &&
           "You tried to access a diffusion grid that does not exist! "
           "Did you specify the correct substance name?");
    return nullptr;
  }

  /// Execute the given functor for all diffusion grids
  ///     rm->ForEachDiffusionGrid([](DiffusionGrid* dgrid) {
  ///       ...
  ///     });
  template <typename TFunctor>
  void ForEachDiffusionGrid(TFunctor&& f) const {
    for (auto& el : diffusion_grids_) {
      f(el.second);
    }
  }

  /// Returns the total number of agents if numa_node == -1
  /// Otherwise the number of agents in the specific numa node
  size_t GetNumAgents(int numa_node = -1) const {
    if (numa_node == -1) {
      size_t num_agents = 0;
      for (auto& numa_agents : agents_) {
        num_agents += numa_agents.size();
      }
      return num_agents;
    } else {
      return agents_[numa_node].size();
    }
  }

  /// Apply a function on all elements in every container
  /// @param function that will be called with each container as a parameter
  ///
  ///     rm->ForEachAgent([](Agent* element) {
  ///                              std::cout << *element << std::endl;
  ///                          });
  virtual void ForEachAgent(const std::function<void(Agent*)>& function) {
    for (auto& numa_agents : agents_) {
      for (auto* agent : numa_agents) {
        function(agent);
      }
    }
  }

  virtual void ForEachAgent(
      const std::function<void(Agent*, AgentHandle)>& function) {
    for (uint64_t n = 0; n < agents_.size(); ++n) {
      auto& numa_agents = agents_[n];
      for (uint64_t i = 0; i < numa_agents.size(); ++i) {
        function(numa_agents[i], AgentHandle(n, i));
      }
    }
  }

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ForEachAgent
  virtual void ForEachAgentParallel(Functor<void, Agent*>& function);

  /// Apply an operation on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ForEachAgent
  virtual void ForEachAgentParallel(Operation& op);

  virtual void ForEachAgentParallel(
      Functor<void, Agent*, AgentHandle>& function);

  /// Apply a function on all elements.\n
  /// Function invocations are parallelized.\n
  /// Uses dynamic scheduling and work stealing. Batch size controlled by
  /// `chunk`.
  /// \param chunk number of agents that are assigned to a thread (batch
  /// size)
  /// \see ForEachAgent
  virtual void ForEachAgentParallel(
      uint64_t chunk, Functor<void, Agent*, AgentHandle>& function);

  /// Reserves enough memory to hold `capacity` number of agents for
  /// each numa domain.
  void Reserve(size_t capacity) {
    for (auto& numa_agents : agents_) {
      numa_agents.reserve(capacity);
    }
    if (type_index_) {
      type_index_->Reserve(capacity);
    }
  }

  /// Resize `agents_[numa_node]` such that it holds `current + additional`
  /// elements after this call.
  /// Returns the size after
  uint64_t GrowAgentContainer(size_t additional, size_t numa_node) {
    if (additional == 0) {
      return agents_[numa_node].size();
    }
    auto current = agents_[numa_node].size();
    if (current + additional < agents_[numa_node].size()) {
      agents_[numa_node].reserve((current + additional) * 1.5);
    }
    agents_[numa_node].resize(current + additional);
    return current;
  }

  /// Returns true if an agent with the given uid is stored in this
  /// ResourceManager.
  bool ContainsAgent(const AgentUid& uid) const {
    return uid_ah_map_.Contains(uid);
  }

  /// Remove all agents
  /// NB: This method is not thread-safe! This function invalidates
  /// agent references pointing into the ResourceManager. AgentPointer are
  /// not affected.
  void ClearAgents() {
    uid_ah_map_.clear();
    for (auto& numa_agents : agents_) {
      for (auto* agent : numa_agents) {
        delete agent;
      }
      numa_agents.clear();
    }
    if (type_index_) {
      type_index_->Clear();
    }
  }

  /// Reorder agents such that, agents are distributed to NUMA
  /// nodes. Nearby agents will be moved to the same NUMA node.
  virtual void LoadBalance();

  void DebugNuma() const;

  /// NB: This method is not thread-safe! This function might invalidate
  /// agent references pointing into the ResourceManager. AgentPointer are
  /// not affected.
  void AddAgent(Agent* agent,  // NOLINT
                typename AgentHandle::NumaNode_t numa_node = 0) {
    auto uid = agent->GetUid();
    if (uid.GetIndex() >= uid_ah_map_.size()) {
      uid_ah_map_.resize(uid.GetIndex() + 1);
    }
    agents_[numa_node].push_back(agent);
    uid_ah_map_.Insert(uid,
                       AgentHandle(numa_node, agents_[numa_node].size() - 1));
    if (type_index_) {
      type_index_->Add(agent);
    }
  }

  void ResizeAgentUidMap() {
    auto* agent_uid_generator = Simulation::GetActive()->GetAgentUidGenerator();
    auto highest_idx = agent_uid_generator->GetHighestIndex();
    auto new_size = highest_idx * 1.5 + 1;
    if (highest_idx >= uid_ah_map_.size()) {
      uid_ah_map_.resize(new_size);
    }
    if (type_index_) {
      type_index_->Reserve(new_size);
    }
  }

  void EndOfIteration() {
    // Check if SoUiD defragmentation should be turned on or off
    double utilization = static_cast<double>(uid_ah_map_.size()) /
                         static_cast<double>(GetNumAgents());
    auto* sim = Simulation::GetActive();
    auto* param = sim->GetParam();
    if (utilization < param->agent_uid_defragmentation_low_watermark) {
      sim->GetAgentUidGenerator()->EnableDefragmentation(&uid_ah_map_);
    } else if (utilization > param->agent_uid_defragmentation_high_watermark) {
      sim->GetAgentUidGenerator()->DisableDefragmentation();
    }
  }

  /// Adds `new_agents` to `agents_[numa_node]`. `offset` specifies
  /// the index at which the first element is inserted. Agents are inserted
  /// consecutively. This methos is thread safe only if insertion intervals do
  /// not overlap!
  virtual void AddAgents(typename AgentHandle::NumaNode_t numa_node,
                         uint64_t offset,
                         const std::vector<Agent*>& new_agents) {
    uint64_t i = 0;
    for (auto* agent : new_agents) {
      auto uid = agent->GetUid();
      uid_ah_map_.Insert(uid, AgentHandle(numa_node, offset + i));
      agents_[numa_node][offset + i] = agent;
      i++;
    }
    if (type_index_) {
#pragma omp critical
      for (auto* agent : new_agents) {
        type_index_->Add(agent);
      }
    }
  }

  /// Removes the agent with the given uid.\n
  /// NB: This method is not thread-safe! This function invalidates
  /// agent references pointing into the ResourceManager. AgentPointer are
  /// not affected.
  void RemoveAgent(const AgentUid& uid) {
    // remove from map
    if (uid_ah_map_.Contains(uid)) {
      auto ah = uid_ah_map_[uid];
      uid_ah_map_.Remove(uid);
      // remove from vector
      auto& numa_agents = agents_[ah.GetNumaNode()];
      Agent* agent = nullptr;
      if (ah.GetElementIdx() == numa_agents.size() - 1) {
        agent = numa_agents.back();
        numa_agents.pop_back();
      } else {
        // swap
        agent = numa_agents[ah.GetElementIdx()];
        auto* reordered = numa_agents.back();
        numa_agents[ah.GetElementIdx()] = reordered;
        numa_agents.pop_back();
        uid_ah_map_.Insert(reordered->GetUid(), ah);
      }
      if (type_index_) {
        type_index_->Remove(agent);
      }
      delete agent;
    }
  }

  const TypeIndex* GetTypeIndex() const { return type_index_; }

 protected:
  /// Maps an AgentUid to its storage location in `agents_` \n
  AgentUidMap<AgentHandle> uid_ah_map_ = AgentUidMap<AgentHandle>(100u);  //!
  /// Pointer container for all agents
  std::vector<std::vector<Agent*>> agents_;
  /// Maps a diffusion grid ID to the pointer to the diffusion grid
  std::unordered_map<uint64_t, DiffusionGrid*> diffusion_grids_;

  ThreadInfo* thread_info_ = ThreadInfo::GetInstance();  //!

  TypeIndex* type_index_ = nullptr;

  friend class SimulationBackup;
  friend std::ostream& operator<<(std::ostream& os, const ResourceManager& rm);
  BDM_CLASS_DEF_NV(ResourceManager, 1);
};

inline std::ostream& operator<<(std::ostream& os, const ResourceManager& rm) {
  os << "\033[1mAgents per numa node\033[0m" << std::endl;
  uint64_t cnt = 0;
  for (auto& numa_agents : rm.agents_) {
    os << "numa node " << cnt++ << " -> size: " << numa_agents.size()
       << std::endl;
  }
  return os;
}

}  // namespace bdm

#endif  // CORE_RESOURCE_MANAGER_H_
