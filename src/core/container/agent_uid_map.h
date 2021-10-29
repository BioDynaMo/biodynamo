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

#ifndef CORE_CONTAINER_AGENT_UID_MAP_H_
#define CORE_CONTAINER_AGENT_UID_MAP_H_

#include <limits>
#include <vector>

#include "core/agent/agent_uid.h"

namespace bdm {

/// AgentUidMap is an associative container that exploits the properties of
/// AgentUid to store data in contigous arrays. Inserting elements and reading
/// elements at the same time is thread-safe as long as the keys are different.
/// These operations with distinct keys are lock-free and atomic free, and thus
/// offer high-performance.
template <typename TValue>
class AgentUidMap {
  struct Iterator {
    AgentUidMap* map_;
    uint64_t idx_;
  };

 public:
  AgentUidMap() {}

  AgentUidMap(const AgentUidMap& other)
      : data_(other.data_), agent_uid_reused_(other.agent_uid_reused_) {}

  explicit AgentUidMap(uint64_t initial_size) {
    data_.resize(initial_size);
    agent_uid_reused_.resize(initial_size, AgentUid::kReusedMax);
  }

  void resize(uint64_t new_size) {  // NOLINT
    data_.resize(new_size);
    agent_uid_reused_.resize(new_size, AgentUid::kReusedMax);
  }

  void clear() {  // NOLINT
    for (auto& el : agent_uid_reused_) {
      el = AgentUid::kReusedMax;
    }
  }

  void ParallelClear() {
#pragma omp parallel for
    for (uint64_t i = 0; i < data_.size(); ++i) {
      agent_uid_reused_[i] = AgentUid::kReusedMax;
    }
  }

  uint64_t size() const {  // NOLINT
    return data_.size();
  }

  void Remove(const AgentUid& key) {
    if (key.GetIndex() >= data_.size()) {
      return;
    }
    agent_uid_reused_[key.GetIndex()] = AgentUid::kReusedMax;
  }

  bool Contains(const AgentUid& uid) const {
    auto idx = uid.GetIndex();
    if (idx >= data_.size()) {
      return false;
    }
    return uid.GetReused() == agent_uid_reused_[idx];
  }

  void Insert(const AgentUid& uid, const TValue& value) {
    auto idx = uid.GetIndex();
    data_[idx] = value;
    agent_uid_reused_[idx] = uid.GetReused();
  }

  const TValue& operator[](const AgentUid& key) const {
    return data_[key.GetIndex()];
  }

  typename AgentUid::Reused_t GetReused(uint64_t index) const {
    return agent_uid_reused_[index];
  }

 private:
  std::vector<TValue> data_;
  std::vector<typename AgentUid::Reused_t> agent_uid_reused_;
};

}  // namespace bdm

#endif  // CORE_CONTAINER_AGENT_UID_MAP_H_
