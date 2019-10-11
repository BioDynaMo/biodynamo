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

#ifndef CORE_CONTAINER_PARALLEL_RESIZE_VECTOR_H_
#define CORE_CONTAINER_PARALLEL_RESIZE_VECTOR_H_

#include <cstdlib>
#include <vector>

namespace bdm {

/// \brief std::vector with parallel resize
template <typename T>
class ParallelResizeVector {
 public:
  using iterator = T*;
  using const_iterator = const T*;
  using value_type = T;

  ParallelResizeVector() {}
  ParallelResizeVector(std::initializer_list<T> init) {
    reserve(init.size());
    for (auto& el : init) {
      push_back(el);
    }
  }

  ParallelResizeVector(const ParallelResizeVector& other) {
    if (other.data_ != nullptr && other.capacity_ != 0) {
      reserve(other.capacity_);
// initialize using copy ctor
#pragma omp parallel for
      for (std::size_t i = 0; i < other.size_; i++) {
        new (&(data_[i])) T(other.data_[i]);
      }
    }
    size_ = other.size_;
    capacity_ = other.capacity_;
  }

  ~ParallelResizeVector() {
    if (data_ != nullptr) {
#pragma omp parallel for
      for (std::size_t i = 0; i < size_; i++) {
        data_[i].~T();
      }
      capacity_ = 0;
      free(data_);
      data_ = nullptr;
    }
  }

  std::size_t size() const { return size_; }  // NOLINT

  T* data() noexcept { return data_; }              // NOLINT
  const T* data() const noexcept { return data_; }  // NOLINT

  void swap(ParallelResizeVector& other) {  // NOLINT
    // size_
    size_ ^= other.size_;
    other.size_ ^= size_;
    size_ ^= other.size_;
    // capacity_
    capacity_ ^= other.capacity_;
    other.capacity_ ^= capacity_;
    capacity_ ^= other.capacity_;
    // data_
    auto* tmp = data_;
    data_ = other.data_;
    other.data_ = tmp;
  }

  std::size_t capacity() const { return capacity_; }  // NOLINT

  void push_back(const T& element) {  // NOLINT
    if (capacity_ == size_) {
      reserve(capacity_ * kGrowFactor);
    }
    new (&(data_[size_++])) T(element);
  }

  void reserve(std::size_t new_capacity) {  // NOLINT
    if (new_capacity > capacity_) {
      T* new_data = static_cast<T*>(malloc(new_capacity * sizeof(T)));
      if (data_ != nullptr) {
// initialize using copy ctor
#pragma omp parallel for
        for (std::size_t i = 0; i < size_; i++) {
          new (&(new_data[i])) T(data_[i]);
        }
// destruct old elements
#pragma omp parallel for
        for (std::size_t i = 0; i < size_; i++) {
          data_[i].~T();
        }
        free(data_);
      }
      data_ = new_data;
      capacity_ = new_capacity;
    }
  }

  void resize(std::size_t new_size, const T& t = T()) {  // NOLINT
    if (capacity_ < new_size) {
      reserve(new_size);
    }

// grow
#pragma omp parallel for
    for (std::size_t i = size_; i < new_size; i++) {
      new (&(data_[i])) T(t);
    }
// shrink
#pragma omp parallel for
    for (std::size_t i = new_size; i < size_; i++) {
      data_[i].~T();
    }
    size_ = new_size;
  }

  void clear() {  // NOLINT
    for (std::size_t i = 0; i < size_; i++) {
      data_[i].~T();
    }
    size_ = 0;
  }

  ParallelResizeVector& operator=(const ParallelResizeVector& other) {
    free(data_);
    data_ = nullptr;
    reserve(other.capacity_);
    size_ = other.size_;
    capacity_ = other.capacity_;

#pragma omp parallel for
    for (std::size_t i = 0; i < size_; i++) {
      data_[i] = other.data_[i];
    }
    return *this;
  }

  T& operator[](std::size_t index) { return data_[index]; }

  const T& operator[](std::size_t index) const { return data_[index]; }

  iterator begin() { return &(data_[0]); }  // NOLINT

  iterator end() { return &(data_[size_]); }  // NOLINT

  const_iterator cbegin() { return &(data_[0]); }  // NOLINT

  const_iterator cend() { return &(data_[size_]); }  // NOLINT

 private:
  static constexpr float kGrowFactor = 1.5;
  std::size_t size_ = 0;
  std::size_t capacity_ = 0;
  T* data_ = nullptr;
};

}  // namespace bdm

#endif  // CORE_CONTAINER_PARALLEL_RESIZE_VECTOR_H_
