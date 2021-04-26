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

#ifndef CORE_CONTAINER_AGENT_FLAT_IDX_MAP_H_
#define CORE_CONTAINER_AGENT_FLAT_IDX_MAP_H_

#include <vector>

#include "core/agent/agent_handle.h"
#include "core/resource_manager.h"
#include "core/simulation.h"
#include "core/util/thread_info.h"

namespace bdm {

/// AgentFlatIdxMap maps flattened agent indices to and from AgentHandles
///
/// NUMA 0: [0 1 2 3]
/// NUMA 1: [0 1 2 3 4]
/// NUMA 2: [0 1 2 3 4 5]
/// ______________________ ↓
///
/// Flattened indices = [0 1 2 3 4 5 6 7 8 9 10 11 12 13 14]
///
/// offset_ = [0, 4, 9]
///
/// AgentFlatIdxMap::GetFlatIdx(AgentHandle(1, 2)) = offset_[1] + 2 = 4 + 2 = 6
/// AgentFlatIdxMap::GetAgentHandle(6) = AgentHandle(1, 2)
///
class AgentFlatIdxMap {
 public:
  AgentFlatIdxMap() {}

  AgentFlatIdxMap(const AgentFlatIdxMap& other) {
    this->offset_ = other.offset_;
  }

  void Update() {
    auto* rm = Simulation::GetActive()->GetResourceManager();
    auto num_numa_nodes = ThreadInfo::GetInstance()->GetNumaNodes();
    offset_.resize(num_numa_nodes);
    offset_[0] = 0;
    for (int nn = 1; nn < num_numa_nodes; nn++) {
      offset_[nn] = offset_[nn - 1] + rm->GetNumAgents(nn - 1);
    }
  }

  uint64_t GetFlatIdx(const AgentHandle& ah) const {
    return offset_[ah.GetNumaNode()] + ah.GetElementIdx();
  }

  AgentHandle GetAgentHandle(uint64_t idx) const {
    size_t nn = 0;
    for (size_t i = 1; i < offset_.size(); i++) {
      if (idx < offset_[i]) {
        break;
      }
      nn++;
    }

    idx -= offset_[nn];
    return AgentHandle(nn, idx);
  }

 private:
  std::vector<size_t> offset_;
};

}  // namespace bdm

#endif  // CORE_CONTAINER_AGENT_FLAT_IDX_MAP_H_
