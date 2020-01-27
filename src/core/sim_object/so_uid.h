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

namespace bdm {

/// SoUid is a unique id for simulation objects that remains unchanged
/// throughout the whole simulation.
using SoUid = uint64_t;

/// This class generates unique ids for simulation objects events satisfying the
/// EventId invariant. Thread safe.
class SoUidGenerator {
 public:
  SoUidGenerator(const SoUidGenerator&) = delete;

  static SoUidGenerator* Get() {
    static SoUidGenerator kInstance;
    return &kInstance;
  }

  SoUid NewSoUid() { return counter_++; }

  SoUid GetLastId() const { return counter_; }

 private:
  SoUidGenerator() : counter_(0) {}
  std::atomic<SoUid> counter_;
};

template <typename TValue>
class SoUidMap {
  struct Iterator {

    SoUidMap* map_;
    uint64_t idx_;
  };

public:
  SoUidMap(const TValue& empty_value, uint64_t initial_size, uint64_t offset = 0) : empty_value_{empty_value}, offset_{offset} {
    data_.resize(initial_size);
  }

  void resize(uint64_t new_size) {  // NOLINT
    data_.resize(new_size, empty_value_);
  }

  void clear() {  // NOLINT
    for (auto& el: data_) {
      el = empty_value_;
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
    if (key - offset_ >= data_.size()) {
      return empty_value_;
    }
    auto previous = data_[key - offset_];
    data_[key - offset_] = empty_value_;
    return previous;
  }

  bool Contains(const SoUid& uid) const {
    if (uid - offset_ >= data_.size()) {
      return false;
    }
    return data_[uid-offset_] != empty_value_;
  }

  void SetOffset(uint64_t offset) {
    offset_ = offset;
  }

  TValue& operator[](const SoUid& key) {
    return data_[key-offset_];
  }

  const TValue& operator[](const SoUid& key) const {
    return data_[key-offset_];
  }

  // find, erase, begin, end

private:
  std::vector<TValue> data_;
  TValue empty_value_;
  uint64_t offset_ = 0;
};

}  // namespace bdm

#endif  // CORE_SIM_OBJECT_SO_UID_H_
