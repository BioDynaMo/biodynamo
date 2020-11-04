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

#ifndef CORE_AGENT_AGENT_HANDLE_H_
#define CORE_AGENT_AGENT_HANDLE_H_

#include <limits>
#include "core/util/root.h"

namespace bdm {

/// Unique identifier of a agent. Acts as a type erased pointer.
/// Has the same type for every agent. \n
/// Points to the storage location of a agent inside ResourceManager.\n
/// The id is split into two parts: Numa node, element index.
/// The first one is used to obtain the numa storage, and the second specifies
/// the element within this vector.
class AgentHandle {
 public:
  using NumaNode_t = uint16_t;
  using ElementIdx_t = uint32_t;

  constexpr AgentHandle() noexcept
      : numa_node_(std::numeric_limits<NumaNode_t>::max()),
        element_idx_(std::numeric_limits<ElementIdx_t>::max()) {}

  explicit AgentHandle(ElementIdx_t element_idx)
      : numa_node_(0), element_idx_(element_idx) {}

  AgentHandle(NumaNode_t numa_node, ElementIdx_t element_idx)
      : numa_node_(numa_node), element_idx_(element_idx) {}

  NumaNode_t GetNumaNode() const { return numa_node_; }
  ElementIdx_t GetElementIdx() const { return element_idx_; }
  void SetElementIdx(ElementIdx_t element_idx) { element_idx_ = element_idx; }

  bool operator==(const AgentHandle& other) const {
    return numa_node_ == other.numa_node_ && element_idx_ == other.element_idx_;
  }

  bool operator!=(const AgentHandle& other) const { return !(*this == other); }

  bool operator<(const AgentHandle& other) const {
    if (numa_node_ == other.numa_node_) {
      return element_idx_ < other.element_idx_;
    } else {
      return numa_node_ < other.numa_node_;
    }
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const AgentHandle& handle) {
    stream << "Numa node: " << handle.numa_node_
           << " element idx: " << handle.element_idx_;
    return stream;
  }

 private:
  NumaNode_t numa_node_;

  /// changed element index to uint32_t after issues with std::atomic with
  /// size 16 -> max element_idx: 4.294.967.296
  ElementIdx_t element_idx_;

  BDM_CLASS_DEF_NV(AgentHandle, 1);
};

}  // namespace bdm

#endif  // CORE_AGENT_AGENT_HANDLE_H_
