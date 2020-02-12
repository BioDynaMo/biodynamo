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

#ifndef CORE_SIM_OBJECT_SO_HANDLE_H_
#define CORE_SIM_OBJECT_SO_HANDLE_H_

#include <limits>
#include "core/util/root.h"

namespace bdm {

/// Unique identifier of a simulation object. Acts as a type erased pointer.
/// Has the same type for every simulation object. \n
/// Points to the storage location of a sim object inside ResourceManager.\n
/// The id is split into two parts: Numa node, element index.
/// The first one is used to obtain the numa storage, and the second specifies
/// the element within this vector.
class SoHandle {
 public:
  using NumaNode_t = uint16_t;
  using ElementIdx_t = uint32_t;

  constexpr SoHandle() noexcept
      : numa_node_(std::numeric_limits<NumaNode_t>::max()),
        element_idx_(std::numeric_limits<ElementIdx_t>::max()) {}

  explicit SoHandle(ElementIdx_t element_idx)
      : numa_node_(0), element_idx_(element_idx) {}

  SoHandle(NumaNode_t numa_node, ElementIdx_t element_idx)
      : numa_node_(numa_node), element_idx_(element_idx) {}

  NumaNode_t GetNumaNode() const { return numa_node_; }
  ElementIdx_t GetElementIdx() const { return element_idx_; }
  void SetElementIdx(ElementIdx_t element_idx) { element_idx_ = element_idx; }

  bool operator==(const SoHandle& other) const {
    return numa_node_ == other.numa_node_ && element_idx_ == other.element_idx_;
  }

  bool operator!=(const SoHandle& other) const { return !(*this == other); }

  bool operator<(const SoHandle& other) const {
    if (numa_node_ == other.numa_node_) {
      return element_idx_ < other.element_idx_;
    } else {
      return numa_node_ < other.numa_node_;
    }
  }

  friend std::ostream& operator<<(std::ostream& stream,
                                  const SoHandle& handle) {
    stream << "Numa node: " << handle.numa_node_
           << " element idx: " << handle.element_idx_;
    return stream;
  }

 private:
  NumaNode_t numa_node_;

  /// changed element index to uint32_t after issues with std::atomic with
  /// size 16 -> max element_idx: 4.294.967.296
  ElementIdx_t element_idx_;

  BDM_CLASS_DEF_NV(SoHandle, 1);
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SO_HANDLE_H_
