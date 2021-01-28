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

#ifndef CORE_CONTAINER_FLATMAP_H_
#define CORE_CONTAINER_FLATMAP_H_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include "core/util/string.h"

namespace bdm {

template <typename TKey, typename TValue>
class UnorderedFlatmap {
 public:
  using value_type = TValue;
  using Pair = std::pair<TKey, TValue>;
  using Iterator = Pair*;
  using ConstIterator = const Pair*;

  UnorderedFlatmap() {}

  void reserve(uint64_t new_capacity) {
    if (new_capacity > size_) {
      data_.resize(new_capacity);
    }
  }

  size_t size() const { return size_; }  // NOLINT

  size_t Capacity() const { return data_.capacity(); }

  void clear() { size_ = 0; }

  const TValue& at(const TKey& key) const {
    auto idx = FindIndexConst(key);
    assert(idx < size_ &&
           Concat("Key (", key, ") not found in UnorderedFlatmap").c_str());
    return data_.at(idx).second;
  }

  void insert(Pair&& pair) {
    if (size_ <= data_.size()) {
      data_.resize((size_ + 1) * 2);
    }
    data_[size_++] = pair;
  }

  TValue& operator[](const TKey& key) { return data_[FindIndex(key)].second; }

  const TValue& operator[](const TKey& key) const {
    return data_[FindIndexConst(key)].second;
  }

  Iterator find(const TKey& key) { return &(data_[FindIndexConst(key)]); }

  ConstIterator find(const TKey& key) const {
    return &(data_[FindIndexConst(key)]);
  }

  Iterator begin() { return &(data_[0]); }
  ConstIterator begin() const { return &(data_[0]); }
  Iterator end() { return &(data_[size_]); }
  ConstIterator end() const { return &(data_[size_]); }

 private:
  std::vector<Pair> data_;
  std::size_t size_ = 0;

  uint64_t FindIndex(const TKey& key) {
    for (uint64_t i = 0; i < size_; i++) {
      if (data_[i].first == key) {
        return i;
      }
    }
    // insert new element
    if (size_ <= data_.size()) {
      data_.resize((size_ + 1) * 2);
    }
    data_[size_].first = key;
    return size_++;
  }

  uint64_t FindIndexConst(const TKey& key) const {
    for (uint64_t i = 0; i < size_; i++) {
      if (data_[i].first == key) {
        return i;
      }
    }
    return size_;
  }
};

}  // namespace bdm

#endif  // CORE_CONTAINER_FLATMAP_H_
