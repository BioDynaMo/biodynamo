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

#ifndef PARALLEL_RESIZE_VECTOR_H_
#define PARALLEL_RESIZE_VECTOR_H_

#include <vector>
#include <iostream>  // FIXME remove

namespace bdm {

/// \brief std::vector with parallel resize
template <typename T>
class ParallelResizeVector {
 public:
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;
  using value_type = T;

  ParallelResizeVector() {}
  ParallelResizeVector(std::initializer_list<T> init)
      : data_(init), size_(init.size()) {}
  ParallelResizeVector(const ParallelResizeVector& other) {
    size_ = other.size_;
    data_.clear();
    data_.reserve(size_);

#pragma omp parallel for
    for (std::size_t i = 0; i < size_; i++) {
      data_[i] = other.data_[i];
    }
  }
  ~ParallelResizeVector() {}

  std::size_t size() const { return size_; }  // NOLINT

  T* data() noexcept { return data_.data(); }              // NOLINT
  const T* data() const noexcept { return data_.data(); }  // NOLINT

  void swap(ParallelResizeVector& other) { data_.swap(other.data_); }  // NOLINT

  std::size_t capacity() const { return data_.capacity(); }  // NOLINT

  void push_back(const T& element) {  // NOLINT
    data_.push_back(element);
    size_++;
  }

  void reserve(std::size_t new_capacity) {  // NOLINT
    data_.reserve(new_capacity);
  }

  void resize(std::size_t new_size, const T& t = T()) {  // NOLINT
    if (data_.capacity() < new_size) {
      data_.reserve(new_size);
    }

#pragma omp parallel for
    for (std::size_t i = size_; i < new_size; i++) {
      data_[i] = t;
    }
    size_ = new_size;
  }

  void clear() {  // NOLINT
    data_.clear();
    size_ = 0;
  }

  ParallelResizeVector& operator=(const ParallelResizeVector& other) {
    size_ = other.size_;
    data_.clear();
    data_.reserve(size_);

#pragma omp parallel for
    for (std::size_t i = 0; i < size_; i++) {
      data_[i] = other.data_[i];
    }
    return *this;
  }

  T& operator[](std::size_t index) { return data_[index]; }

  const T& operator[](std::size_t index) const { return data_[index]; }

  iterator begin() { return data_.begin(); }  // NOLINT

  iterator end() { return data_.begin() += size_; }  // NOLINT

  const_iterator cbegin() { return data_.cbegin(); }  // NOLINT

  const_iterator cend() { return data_.cbegin() += size_; }  // NOLINT

 private:
  std::vector<T> data_;
  std::size_t size_ = 0;
};

}  // namespace bdm

#endif  // PARALLEL_RESIZE_VECTOR_H_
