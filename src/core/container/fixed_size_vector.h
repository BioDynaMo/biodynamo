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

#ifndef CORE_CONTAINER_FIXED_SIZE_VECTOR_H_
#define CORE_CONTAINER_FIXED_SIZE_VECTOR_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>

namespace bdm {

/// Vector with fixed number of elements == Array with push_back function that
/// keeps track of its size
/// NB: No bounds checking. Do not push_back more often than the number of
/// maximum elements given by the template parameter N
template <typename T, std::size_t N>
class FixedSizeVector {
 public:
  FixedSizeVector() {}

  /// Constructor which accepts an std::initiliazer_list to set
  /// the array's content.
  /// \param l an initializer list
  constexpr FixedSizeVector(const std::initializer_list<T>& l) {
    assert(l.size() <= N);
    auto it = l.begin();
    for (size_t i = 0; i < N; i++) {
      data_[i] = *(it++);
    }
    size_ = l.size();
  }

  size_t size() const { return size_; }  // NOLINT

  const T& operator[](size_t idx) const {
    assert(idx < N && "Out of bounds access.");
    return data_[idx];
  }

  T& operator[](size_t idx) {
    assert(idx < N && "Out of bounds access.");
    return data_[idx];
  }

  bool operator==(const FixedSizeVector& other) const {
    if (size_ != other.size_) {
      return false;
    }
    for (size_t i = 0; i < std::min(size_, other.size_); i++) {
      if (data_[i] != other.data_[i]) {
        return false;
      }
    }
    return true;
  }

  FixedSizeVector& operator++() {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      ++data_[i];
    }
    return *this;
  }

  void clear() { size_ = 0; }  // NOLINT

  void push_back(const T& value) {  // NOLINT
    assert(size_ < N);
    data_[size_++] = value;
  }

  const T* data() const { return data_; }

  const T* begin() const { return &(data_[0]); }    // NOLINT
  const T* end() const { return &(data_[size_]); }  // NOLINT
  T* begin() { return &(data_[0]); }                // NOLINT
  T* end() { return &(data_[size_]); }              // NOLINT

 private:
  T data_[N];
  std::size_t size_ = 0;
};

}  // namespace bdm

#endif  // CORE_CONTAINER_FIXED_SIZE_VECTOR_H_
