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

#ifndef CORE_AGENT_AGENT_UID_H_
#define CORE_AGENT_AGENT_UID_H_

#include <limits>
#include "core/util/root.h"

namespace bdm {

/// AgentUid is a unique id for agents that remains unchanged
/// throughout the whole simulation.
class AgentUid {
 public:
  using Index_t = uint32_t;
  using Reused_t = uint32_t;
  friend std::hash<AgentUid>;

  static constexpr Reused_t kReusedMax = std::numeric_limits<Reused_t>::max();

  constexpr AgentUid() noexcept
      : index_(std::numeric_limits<Index_t>::max()),
        reused_(std::numeric_limits<Reused_t>::max()) {}

  explicit AgentUid(Index_t index) : index_(index), reused_(0) {}

  AgentUid(Index_t idx, Reused_t reused) : index_(idx), reused_(reused) {}

  Reused_t GetReused() const { return reused_; }
  Index_t GetIndex() const { return index_; }

  // Copy constructors needed for nanoflann kd-tree implementation
  AgentUid(size_t idx) : index_(idx), reused_(0) {}
  AgentUid(int idx) : index_(idx), reused_(0) {}

  // Operators needed for nanoflann kd-tree implementation
  void operator++() { ++index_; }
  void operator--() { --index_; }

  bool operator==(const AgentUid& other) const {
    return index_ == other.index_ && reused_ == other.reused_;
  }

  bool operator!=(const AgentUid& other) const { return !(*this == other); }

  bool operator<(const AgentUid& other) const {
    if (reused_ == other.reused_) {
      return index_ < other.index_;
    } else {
      return reused_ < other.reused_;
    }
  }

  bool operator<(size_t other) const { return index_ < other; }

  AgentUid operator+(int i) const {
    AgentUid uid(*this);
    uid.index_ += i;
    return uid;
  }

  AgentUid operator+(uint64_t i) const {
    AgentUid uid(*this);
    uid.index_ += i;
    return uid;
  }

  AgentUid operator-(int i) const {
    AgentUid uid(*this);
    uid.index_ -= i;
    return uid;
  }

  AgentUid operator-(uint64_t i) const {
    AgentUid uid(*this);
    uid.index_ -= i;
    return uid;
  }

  AgentUid& operator+=(const AgentUid& uid) {
    index_ += uid.index_;
    return *this;
  }

  operator uint64_t() const {
    return (static_cast<uint64_t>(reused_) << 32) |
           static_cast<uint64_t>(index_);
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const AgentUid& handle) {
    stream << handle.index_ << "-" << handle.reused_;
    return stream;
  }

 private:
  /// Consistent with AgentHandle::Index_t
  /// -> max element_idx: 4.294.967.296
  Index_t index_;

  /// Determines how often index_ has been resused
  Reused_t reused_;

  BDM_CLASS_DEF_NV(AgentUid, 1);
};

}  // namespace bdm

namespace std {

template <>
struct hash<bdm::AgentUid> {
  std::size_t operator()(const bdm::AgentUid& uid) const noexcept {
    // at any given time in a simulation there exists only one
    // agent with a specific index_ value.
    // Therefore it is sufficient as a hash.
    // This might change for the distributed runtime.
    return uid.index_;
  }
};

}  // namespace std

#endif  // CORE_AGENT_AGENT_UID_H_
