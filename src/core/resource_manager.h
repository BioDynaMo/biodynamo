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

#include "core/agent/agent.h"
#include "core/agent/agent_handle.h"
#include "core/agent/agent_uid.h"
#include "core/agent/agent_uid_generator.h"
#include "core/container/agent_uid_map.h"
#include "core/diffusion/diffusion_grid.h"
#include "core/functor.h"
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

  virtual ~ResourceManager();

  ResourceManager& operator=(ResourceManager&& other) noexcept {
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
    agents_lb_.resize(agents_.size());
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
    for (AgentHandle::NumaNode_t n = 0; n < agents_.size(); ++n) {
      for (AgentHandle::ElementIdx_t i = 0; i < agents_[n].size(); ++i) {
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

  void SwapAgents(std::vector<std::vector<Agent*>>* agents);

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
    MarkEnvironmentOutOfSync();
  }

  void RemoveDiffusionGrid(size_t substance_id) {
    auto search = diffusion_grids_.find(substance_id);
    if (search != diffusion_grids_.end()) {
      delete search->second;
      diffusion_grids_.erase(search);
    } else {
      Log::Error("ResourceManager::RemoveDiffusionGrid",
                 "You tried to remove a diffusion grid that does not exist.");
    }
  }

  /// Return the diffusion grid which holds the substance of specified id
  DiffusionGrid* GetDiffusionGrid(size_t substance_id) const {
    if (substance_id >= diffusion_grids_.size()) {
      Log::Error("ResourceManager::GetDiffusionGrid",
                 "You tried to request diffusion grid '", substance_id,
                 "', but it does not exist! Make sure that it's the correct id "
                 "correctly and that the diffusion grid is registered.");
      return nullptr;
    }
    return diffusion_grids_.at(substance_id);
  }

  /// Return the diffusion grid which holds the substance of specified name
  /// Caution: using this function in a tight loop will result in a slow
  /// simulation. Use `GetDiffusionGrid(size_t)` in those cases.
  DiffusionGrid* GetDiffusionGrid(const std::string& substance_name) const {
    for (auto& el : diffusion_grids_) {
      auto& dgrid = el.second;
      if (dgrid->GetSubstanceName() == substance_name) {
        return dgrid;
      }
    }
    Log::Error("ResourceManager::GetDiffusionGrid",
               "You tried to request a diffusion grid named '", substance_name,
               "', but it does not exist! Make sure that it's spelled "
               "correctly and that the diffusion grid is registered.");
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

  size_t GetAgentVectorCapacity(int numa_node);

  /// Call a function for all or a subset of agents in the simulation.
  /// @param function that will be called for each agent
  /// @param filter if specified, `function` will only be called for agents
  ///               for which `filter(agent)` evaluates to true.
  ///
  ///     rm->ForEachAgent([](Agent* a) {
  ///                              std::cout << a->GetUid() << std::endl;
  ///                          });
  virtual void ForEachAgent(const std::function<void(Agent*)>& function,
                            Functor<bool, Agent*>* filter = nullptr) {
    for (auto& numa_agents : agents_) {
      for (auto* agent : numa_agents) {
        if (!filter || (filter && (*filter)(agent))) {
          function(agent);
        }
      }
    }
  }

  virtual void ForEachAgent(
      const std::function<void(Agent*, AgentHandle)>& function,
      Functor<bool, Agent*>* filter = nullptr) {
    for (AgentHandle::NumaNode_t n = 0; n < agents_.size(); ++n) {
      auto& numa_agents = agents_[n];
      for (AgentHandle::ElementIdx_t i = 0; i < numa_agents.size(); ++i) {
        auto* a = numa_agents[i];
        if (!filter || (filter && (*filter)(a))) {
          function(a, AgentHandle(n, i));
        }
      }
    }
  }

  /// Call a function for all or a subset of agents in the simulation.
  /// @param function that will be called for each agent
  /// @param filter if specified, `function` will only be called for agents
  ///               for which `filter(agent)` evaluates to true.
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ForEachAgent
  virtual void ForEachAgentParallel(Functor<void, Agent*>& function,
                                    Functor<bool, Agent*>* filter = nullptr);

  /// Call an operation for all or a subset of agents in the simulation.
  /// Function invocations are parallelized.\n
  /// Uses static scheduling.
  /// \see ForEachAgent
  virtual void ForEachAgentParallel(Operation& op,
                                    Functor<bool, Agent*>* filter = nullptr);

  virtual void ForEachAgentParallel(
      Functor<void, Agent*, AgentHandle>& function,
      Functor<bool, Agent*>* filter = nullptr);

  /// Call a function for all or a subset of agents in the simulation.
  /// Function invocations are parallelized.\n
  /// Uses dynamic scheduling and work stealing. Batch size controlled by
  /// `chunk`.
  /// \param chunk number of agents that are assigned to a thread (batch
  /// size)
  /// \see ForEachAgent
  virtual void ForEachAgentParallel(
      uint64_t chunk, Functor<void, Agent*, AgentHandle>& function,
      Functor<bool, Agent*>* filter = nullptr);

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
    uid_ah_map_.Insert(
        uid, AgentHandle(numa_node, static_cast<AgentHandle::ElementIdx_t>(
                                        agents_[numa_node].size() - 1u)));
    if (type_index_) {
      type_index_->Add(agent);
    }
    MarkEnvironmentOutOfSync();
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

  virtual void EndOfIteration() {
    // Check if SoUiD defragmentation should be turned on or off
    real_t utilization = static_cast<real_t>(GetNumAgents()) /
                         static_cast<real_t>(uid_ah_map_.size());
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
      uid_ah_map_.Insert(
          uid, AgentHandle(numa_node,
                           static_cast<AgentHandle::ElementIdx_t>(offset + i)));
      agents_[numa_node][offset + i] = agent;
      i++;
    }
    if (type_index_) {
#pragma omp critical
      for (auto* agent : new_agents) {
        type_index_->Add(agent);
      }
    }
#pragma omp single
    if (new_agents.size() != 0) {
      MarkEnvironmentOutOfSync();
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
      MarkEnvironmentOutOfSync();
    }
  }

  // \param uids: one vector for each thread containing one vector for each numa
  //              node
  void RemoveAgents(const std::vector<std::vector<AgentUid>*>& uids);

  const TypeIndex* GetTypeIndex() const { return type_index_; }

 protected:
  /// Adding and removing agents does not immediately reflect in the state of
  /// the environment. This function sets a flag in the envrionment such that
  /// it is aware of the changes.
  void MarkEnvironmentOutOfSync();

  /// Maps an AgentUid to its storage location in `agents_` \n
  AgentUidMap<AgentHandle> uid_ah_map_ = AgentUidMap<AgentHandle>(100u);  //!
  /// Pointer container for all agents
  std::vector<std::vector<Agent*>> agents_;
  /// Container used during load balancing
  std::vector<std::vector<Agent*>> agents_lb_;  //!
  /// Maps a diffusion grid ID to the pointer to the diffusion grid
  std::unordered_map<uint64_t, DiffusionGrid*> diffusion_grids_;

  ThreadInfo* thread_info_ = ThreadInfo::GetInstance();  //!

  TypeIndex* type_index_ = nullptr;

  struct ParallelRemovalAuxData {
    std::vector<std::vector<uint64_t>> to_right;
    std::vector<std::vector<uint64_t>> not_to_left;
  };

  /// auxiliary data required for parallel agent removal
  ParallelRemovalAuxData parallel_remove_;  //!

  friend class SimulationBackup;
  friend std::ostream& operator<<(std::ostream& os, const ResourceManager& rm);
  BDM_CLASS_DEF_NV(ResourceManager, 2);
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
