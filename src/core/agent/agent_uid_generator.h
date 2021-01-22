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

#ifndef CORE_AGENT_AGENT_UID_GENERATOR_H_
#define CORE_AGENT_AGENT_UID_GENERATOR_H_

#include <atomic>
#include <limits>
#include <mutex>
#include "core/agent/agent_handle.h"
#include "core/agent/agent_uid.h"
#include "core/container/agent_uid_map.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"

namespace bdm {

/// This class generates unique ids for agents.
/// All functions are  thread safe.
class AgentUidGenerator {
 public:
  AgentUidGenerator(const AgentUidGenerator&) = delete;
  AgentUidGenerator() : counter_(0) {}

  /// Generates AgentUid with increasing index.
  /// In defragmentation mode it resuses index values from removed agents
  /// and sets the reused field to the current simulation step.
  AgentUid GenerateUid() {
    if (map_ != nullptr) {
      // defragmentation mode
      std::lock_guard<Spinlock> guard(lock_);
      // repeat check, another thread might have disabled defragmentation
      if (map_ != nullptr) {
        // find unused element in map
        while (search_index_ < map_->size() &&
               map_->GetReused(search_index_) !=
                   std::numeric_limits<typename AgentUid::Reused_t>::max()) {
          search_index_++;
        }
        if (search_index_ < map_->size()) {
          auto* scheduler = Simulation::GetActive()->GetScheduler();
          return AgentUid(search_index_++, scheduler->GetSimulatedSteps());
        }
        // didn't find any empty slots -> disable defragmentation mode
        DisableDefragmentation();
      }
    }
    return AgentUid(counter_++);
  }

  // Returns the highest index that was used for an AgentUid
  uint64_t GetHighestIndex() const { return counter_; }

  void EnableDefragmentation(const AgentUidMap<AgentHandle>* map) {
    // check if already in defragmentation mode
    if (map_ == nullptr) {
      map_ = map;
      search_index_ = 0;
    }
  }

  void DisableDefragmentation() {
    if (map_ != nullptr) {
      counter_ = map_->size();
    }
    map_ = nullptr;
  }

  bool IsInDefragmentationMode() const { return map_ != nullptr; }

 private:
  std::atomic<typename AgentUid::Index_t> counter_;  //!
  /// ROOT can't persist std::atomic.
  /// Therefore this additional helper variable is needed.
  typename AgentUid::Index_t root_counter_;

  ///
  const AgentUidMap<AgentHandle>* map_ = nullptr;  //!
  /// Lock needed for defragmentation mode
  Spinlock lock_;  //!
  /// Current search position
  typename AgentUid::Index_t search_index_ = 0;  //!
  BDM_CLASS_DEF_NV(AgentUidGenerator, 1);
};

// The following custom streamer should be visible to rootcling for dictionary
// generation, but not to the interpreter!
#if (!defined(__CLING__) || defined(__ROOTCLING__)) && defined(USE_DICT)

inline void AgentUidGenerator::Streamer(TBuffer& R__b) {
  if (R__b.IsReading()) {
    R__b.ReadClassBuffer(AgentUidGenerator::Class(), this);
    this->counter_ = this->root_counter_;
  } else {
    this->root_counter_ = this->counter_.load();
    R__b.WriteClassBuffer(AgentUidGenerator::Class(), this);
  }
}

#endif  // !defined(__CLING__) || defined(__ROOTCLING__)

}  // namespace bdm

#endif  // CORE_AGENT_AGENT_UID_GENERATOR_H_
