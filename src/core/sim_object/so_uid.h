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

#include <atomic>
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



/// This class generates unique ids for simulation objects events satisfying the
/// EventId invariant. Thread safe.
class SoUidGenerator {
 public:
  SoUidGenerator(const SoUidGenerator&) = delete;

  static SoUidGenerator* Get() {
    static SoUidGenerator kInstance;
    return &kInstance;
  }

  SoUid NewSoUid() { return SoUid(counter_++); }

  SoUid GetLastId() const { return SoUid(counter_); }

 private:
  SoUidGenerator() : counter_(0) {}
  std::atomic<typename SoUid::Index_t> counter_;
};

template <typename TValue>
class SoUidMap {
  struct Iterator {

    SoUidMap* map_;
    uint64_t idx_;
  };

public:
  SoUidMap(const TValue& empty_value, uint64_t initial_size) : empty_value_{empty_value} {
    data_.resize(initial_size);
    so_uid_reused_.resize(initial_size, SoUid::kReusedMax);
  }

  void resize(uint64_t new_size) {  // NOLINT
    data_.resize(new_size, empty_value_);
    so_uid_reused_.resize(new_size, SoUid::kReusedMax);
  }

  void clear() {  // NOLINT
    for (auto& el: data_) {
      el = empty_value_;el = empty_value_; // FIXME is this needed??
    }
    for (auto& el: so_uid_reused_) {
      el = SoUid::kReusedMax;
    }
  }

  void ParallelClear() {
    #pragma omp parallel for
    for (uint64_t i = 0; i < data_.size(); ++i) {
      data_[i] = empty_value_;
    }
  }

  uint64_t size() const {  // NOLINT
    return data_.size();
  }

  TValue Remove(const SoUid& key) {
    if (key.GetIndex() >= data_.size()) {
      return empty_value_;
    }
    auto previous = data_[key.GetIndex()];
    data_[key.GetIndex()] = empty_value_;  // FIXME is this needed??
    so_uid_reused_[key.GetIndex()] = key.GetReused();
    return previous;
  }

  bool Contains(const SoUid& uid) const {
    if (uid.GetIndex() >= data_.size()) {
      return false;
    }
    // FIXME is comparison with empty value needed??
    return data_[uid.GetIndex()] != empty_value_
      && so_uid_reused_[uid.GetReused()] != SoUid::kReusedMax;
  }

  TValue& operator[](const SoUid& key) {
    return data_[key.GetIndex()];
  }

  const TValue& operator[](const SoUid& key) const {
    return data_[key.GetIndex()];
  }

  // find, erase, begin, end

private:
  std::vector<TValue> data_;
  std::vector<typename SoUid::Reused_t> so_uid_reused_;
  TValue empty_value_;
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
