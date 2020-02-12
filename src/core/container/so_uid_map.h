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

#ifndef CORE_CONTAINER_SO_UID_MAP_H_
#define CORE_CONTAINER_SO_UID_MAP_H_

#include <limits>
#include <vector>

#include "core/sim_object/so_uid.h"

namespace bdm {

/// SoUidMap is an associative container that exploits the properties of SoUid
/// to store data in contigous arrays.
/// Inserting elements and reading elements at the same time is thread-safe
/// as long as the keys are different.
/// These operations with distinct keys are lock-free and atomic free, and thus
/// offer high-performance.
template <typename TValue>
class SoUidMap {
  struct Iterator {
    SoUidMap* map_;
    uint64_t idx_;
  };

 public:
  SoUidMap() {}

  SoUidMap(const SoUidMap& other)
      : data_(other.data_), so_uid_reused_(other.so_uid_reused_) {}

  explicit SoUidMap(uint64_t initial_size) {
    data_.resize(initial_size);
    so_uid_reused_.resize(initial_size, SoUid::kReusedMax);
  }

  void resize(uint64_t new_size) {  // NOLINT
    data_.resize(new_size);
    so_uid_reused_.resize(new_size, SoUid::kReusedMax);
  }

  void clear() {  // NOLINT
    for (auto& el : so_uid_reused_) {
      el = SoUid::kReusedMax;
    }
  }

  void ParallelClear() {
#pragma omp parallel for
    for (uint64_t i = 0; i < data_.size(); ++i) {
      so_uid_reused_[i] = SoUid::kReusedMax;
    }
  }

  uint64_t size() const {  // NOLINT
    return data_.size();
  }

  void Remove(const SoUid& key) {
    if (key.GetIndex() >= data_.size()) {
      return;
    }
    so_uid_reused_[key.GetIndex()] = SoUid::kReusedMax;
  }

  bool Contains(const SoUid& uid) const {
    auto idx = uid.GetIndex();
    if (idx >= data_.size()) {
      return false;
    }
    return uid.GetReused() == so_uid_reused_[idx];
  }

  void Insert(const SoUid& uid, const TValue& value) {
    auto idx = uid.GetIndex();
    data_[idx] = value;
    so_uid_reused_[idx] = uid.GetReused();
  }

  const TValue& operator[](const SoUid& key) const {
    return data_[key.GetIndex()];
  }

  typename SoUid::Reused_t GetReused(uint64_t index) const {
    return so_uid_reused_[index];
  }

 private:
  std::vector<TValue> data_;
  std::vector<typename SoUid::Reused_t> so_uid_reused_;
};

}  // namespace bdm

#endif  // CORE_CONTAINER_SO_UID_MAP_H_
