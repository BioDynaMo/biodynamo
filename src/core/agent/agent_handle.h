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

#ifndef CORE_AGENT_AGENT_HANDLE_H_
#define CORE_AGENT_AGENT_HANDLE_H_

#include <limits>
#include "core/util/root.h"

namespace bdm {

/// Unique identifier of an agent. Acts as a type erased pointer.
/// Has the same type for every agent. \n
/// Points to the storage location of an agent inside ResourceManager.\n
/// The id is split into two parts: Numa node, element index.
/// The first one is used to obtain the numa storage, and the second specifies
/// the element within this vector.
class AgentHandle {
 public:
  using PrimaryIndex_t = uint16_t;
  using SecondaryIndex_t = uint32_t;

  constexpr AgentHandle() noexcept
      : primary_idx_(std::numeric_limits<PrimaryIndex_t>::max()),
        secondary_index_(std::numeric_limits<SecondaryIndex_t>::max()) {}

  explicit AgentHandle(SecondaryIndex_t secondary_index)
      : primary_idx_(0), secondary_index_(secondary_index) {}

  AgentHandle(PrimaryIndex_t primary_idx, SecondaryIndex_t secondary_index)
      : primary_idx_(primary_idx), secondary_index_(secondary_index) {}

  AgentHandle(bool in_aura, PrimaryIndex_t primary_idx,
              SecondaryIndex_t secondary_index)
      : in_aura_(in_aura),
        primary_idx_(primary_idx),
        secondary_index_(secondary_index) {}

  // TODO deprecate
  PrimaryIndex_t GetNumaNode() const { return primary_idx_; }
  SecondaryIndex_t GetElementIdx() const { return secondary_index_; }
  void SetElementIdx(SecondaryIndex_t secondary_index) {
    secondary_index_ = secondary_index;
  }
  // TODO deprecate end

  bool IsInAura() const { return in_aura_; }
  PrimaryIndex_t GetPrimaryIndex() const { return primary_idx_; }
  SecondaryIndex_t GetSecondaryIndex() const { return secondary_index_; }
  void SetSecondaryIndex(SecondaryIndex_t secondary_index) {
    secondary_index_ = secondary_index;
  }

  bool operator==(const AgentHandle& other) const {
    return in_aura_ == other.in_aura_ && primary_idx_ == other.primary_idx_ &&
           secondary_index_ == other.secondary_index_;
  }

  bool operator!=(const AgentHandle& other) const { return !(*this == other); }

  bool operator<(const AgentHandle& other) const {
    if (in_aura_ == other.in_aura_ && primary_idx_ == other.primary_idx_) {
      return secondary_index_ < other.secondary_index_;
    } else if (in_aura_ == other.in_aura_) {
      return primary_idx_ < other.primary_idx_;
    } else {
      return !in_aura_;
    }
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const AgentHandle& handle) {
    stream << "in aura: " << handle.in_aura_
           << " primary index: " << handle.primary_idx_
           << " secondary index: " << handle.secondary_index_;
    return stream;
  }

 private:
  bool in_aura_ = false;
  PrimaryIndex_t primary_idx_;
  SecondaryIndex_t secondary_index_;

  BDM_CLASS_DEF_NV(AgentHandle, 2);
};

}  // namespace bdm

#endif  // CORE_AGENT_AGENT_HANDLE_H_
