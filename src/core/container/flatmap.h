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

#ifndef CORE_CONTAINER_UNORDERED_FLATMAP_H_
#define CORE_CONTAINER_UNORDERED_FLATMAP_H_

#include <cassert>
#include <cstddef>
#include <vector>
#include <utility>

namespace bdm {

template <typename TKey, typename TValue>
class unordered_flatmap {
 public:
   using value_type = TValue;
   using Iterator = std::pair<TKey, TValue>*;
   using ConstIterator = const std::pair<TKey, TValue>*;

   unordered_flatmap() {}

   void reserve(uint64_t new_capacity) { data_.resize(new_capacity); }

  size_t size() const { return size_; }  // NOLINT

  void clear() { size_ = 0; }

  const TValue& at(const TKey& key) const {
    // FIXME throw exception if element has not been found
    return data_.at(FindIndex(key)).second;
  }

  TValue& operator[](const TKey& key) { return data_[FindIndex(key)].second; }

  Iterator find(const TKey& key) { return &(data_[FindIndex(key)]); }

  ConstIterator find(const TKey& key) const { return &(data_[FindIndex(key)]); }

  Iterator begin() { return &(data_[0]); }
  ConstIterator begin() const { return &(data_[0]); }
  Iterator end() { return &(data_[size_]); }
  ConstIterator end() const { return &(data_[size_]); }

 private:
  std::vector<std::pair<TKey, TValue>> data_;
  std::size_t size_ = 0;

  uint64_t FindIndex(const TKey& key) {
    for (int i = 0; i < size_; i++) {
      if (data_[i].first == key) {
        return i;
      }
    }
    // insert new element
    // FIXME make sure there is enough space
    data_[size_].first = key;
    return size_++;
  }

  uint64_t FindIndex(const TKey& key) const {
    for (int i = 0; i < size_; i++) {
      if (data_[i].first == key) {
        return i;
      }
    }
    return size_;
  }
};

}  // namespace bdm

#endif  // CORE_CONTAINER_UNORDERED_FLATMAP_H_
