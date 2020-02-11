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

#ifndef CORE_SIM_OBJECT_SO_UID_H_
#define CORE_SIM_OBJECT_SO_UID_H_

#include <limits>
#include "core/util/root.h"

namespace bdm {

/// SoUid is a unique id for simulation objects that remains unchanged
/// throughout the whole simulation.
class SoUid {
 public:
  using Index_t = uint32_t;
  using Reused_t = uint32_t;
  friend std::hash<SoUid>;

  static constexpr Reused_t kReusedMax = std::numeric_limits<Reused_t>::max();

  constexpr SoUid() noexcept
      : index_(std::numeric_limits<Index_t>::max()),
        reused_(std::numeric_limits<Reused_t>::max()) {}

  explicit SoUid(Index_t index)
      : index_(index), reused_(0) {}

  SoUid(Index_t idx, Reused_t reused)
      : index_(idx), reused_(reused) {}

  Reused_t GetReused() const { return reused_; }
  Index_t GetIndex() const { return index_; }

  bool operator==(const SoUid& other) const {
    return index_ == other.index_ && reused_ == other.reused_;
  }

  bool operator!=(const SoUid& other) const { return !(*this == other); }

  bool operator<(const SoUid& other) const {
    if (reused_ == other.reused_) {
      return index_ < other.index_;
    } else {
      return reused_ < other.reused_;
    }
  }

  SoUid operator+(int i) const {
    SoUid uid(*this);
    uid.index_ += i;
    return uid;
  }

  SoUid operator+(uint64_t i) const {
    SoUid uid(*this);
    uid.index_ += i;
    return uid;
  }

  SoUid operator-(int i) const {
    SoUid uid(*this);
    uid.index_ -= i;
    return uid;
  }

  SoUid operator-(uint64_t i) const {
    SoUid uid(*this);
    uid.index_ -= i;
    return uid;
  }

  SoUid& operator+=(const SoUid& uid) {
    index_ += uid.index_;
    return *this;
  }

  operator uint64_t() const {
    return (static_cast<uint64_t>(reused_) << 32) | static_cast<uint64_t>(index_);
  }

  friend std::ostream& operator<<(std::ostream& stream, const SoUid& handle) {
    stream << handle.index_ << "-" << handle.reused_;
    return stream;
  }

 private:
  /// Consistent with SoHandle::Index_t
  /// -> max element_idx: 4.294.967.296
  Index_t index_;

  /// Determines how often index_ has been resused
  Reused_t reused_;

  // TODO delete assignement operator and copy ctor

  BDM_CLASS_DEF_NV(SoUid, 1);
};

}  // namespace bdm

namespace std {

template<>
struct hash<bdm::SoUid> {
  std::size_t operator()(const bdm::SoUid& uid) const noexcept {
      return (uid.index_) << uid.reused_;
  }
};

}  // namespace std

#endif  // CORE_SIM_OBJECT_SO_UID_H_
