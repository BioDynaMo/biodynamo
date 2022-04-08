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

#ifndef CORE_AGENT_AGENT_UID_GENERATOR_H_
#define CORE_AGENT_AGENT_UID_GENERATOR_H_

#include <atomic>
#include <limits>
#include <mutex>
#include "core/agent/agent_handle.h"
#include "core/agent/agent_uid.h"
#include "core/container/agent_uid_map.h"
#include "core/container/shared_data.h"
#include "core/scheduler.h"
#include "core/simulation.h"
#include "core/util/root.h"
#include "core/util/spinlock.h"

namespace bdm {

/// This class generates unique ids for agents.
class AgentUidGenerator {
 public:
  AgentUidGenerator(const AgentUidGenerator&) = delete;
  AgentUidGenerator() : counter_(0), tinfo_(ThreadInfo::GetInstance()) {
    Update();
  }

  /// Generates AgentUid with increasing index.
  /// In defragmentation mode it reuses index values from removed agents
  /// and increments the reused field.
  /// Thread-safe.
  AgentUid GenerateUid() {
    auto& old_uids = tl_uids_[tinfo_->GetMyThreadId()];
    if (old_uids.size()) {
      auto uid = old_uids.back();
      old_uids.pop_back();
      return AgentUid(uid.GetIndex(), uid.GetReused() + 1);
    }
    return AgentUid(counter_++);
  }

  // Returns the highest index that was used for an AgentUid
  /// Thread-safe.
  AgentUid::Index_t GetHighestIndex() const { return counter_; }

  /// Adds AgentUid that can be reused after AgentUid::reused_ is incremented.
  /// Thread-safe.
  void ReuseAgentUid(const AgentUid& uid) {
    tl_uids_[tinfo_->GetMyThreadId()].push_back(uid);
  }

  /// Resizes internal data structures to the number of threads.
  /// NB: If Update is called, calls to GenerateUid or ReuseAgentUid are not
  /// allowed!
  void Update() { tl_uids_.resize(tinfo_->GetMaxThreads()); }

 private:
  std::atomic<typename AgentUid::Index_t> counter_;  //!
  /// ROOT can't persist std::atomic.
  /// Therefore this additional helper variable is needed.
  typename AgentUid::Index_t root_counter_;

  /// Thread local vector of AgentUids that can be reused
  SharedData<std::vector<AgentUid>> tl_uids_;
  ThreadInfo* tinfo_ = nullptr;  //!

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
