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

#ifndef CORE_CONTAINER_SHARED_DATA_H_
#define CORE_CONTAINER_SHARED_DATA_H_

#include <array>
#include <vector>

#define BDM_CACHE_LINE_SIZE 64

namespace bdm {

// This class avoids false sharing between threads
template <typename T>
class SharedData {
 public:
  using Data = std::vector<std::array<T, BDM_CACHE_LINE_SIZE / sizeof(T)>>;
  SharedData() {}
  SharedData(size_t size, const T& value = T()) {
    data_.resize(size);
    for (auto& arr : data_) {
      arr[0] = value;
    }
  }
  T& operator[](size_t index) { return data_[index][0]; }
  const T& operator[](size_t index) const { return data_[index][0]; }
  size_t size() const { return data_.size(); }  // NOLINT
  void resize(size_t new_size) {                // NOTLINT
    data_.resize(new_size);
  }

  struct Iterator {
    uint64_t index;
    Data* data;
    Iterator& operator++() {
      ++index;
      return *this;
    }
    bool operator==(const Iterator& other) {
      return index == other.index && data == other.data;
    }
    bool operator!=(const Iterator& other) { return !operator==(other); }
    T& operator*() { return (*data)[index][0]; }
    const T& operator*() const { return (*data)[index][0]; }
  };

  Iterator begin() { return Iterator{0, &data_}; }
  Iterator end() { return Iterator{data_.size(), &data_}; }
  const Iterator begin() const {
    return Iterator{0, const_cast<Data*>(&data_)};
  }
  const Iterator end() const {
    return Iterator{data_.size(), const_cast<Data*>(&data_)};
  }

 private:
  Data data_;
};

}  // namespace bdm

#endif  // CORE_CONTAINER_SHARED_DATA_H_
