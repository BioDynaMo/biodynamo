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

#ifndef CORE_CONTAINER_INLINE_VECTOR_H_
#define CORE_CONTAINER_INLINE_VECTOR_H_

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
#include <sstream>
#include <vector>

#include "core/util/log.h"
#include "core/util/root.h"

namespace bdm {

/// This containes stores up to N elements without heap allocations
/// If further elements are added elements are stored on the heap
/// Container grows in a geometric sequence
/// Elements are contiguous in memory exept the transition from internal
/// to heap allocated memory (between index N-1 and N)
/// This container is optimized for minimal overhead. Therefore, it can only
/// store 65535 elements.
template <typename T, uint16_t N>
class InlineVector {
 public:
  template <typename TT, typename TIV>
  struct Iterator {
    uint16_t index_ = 0;
    TIV* vector_;

    Iterator(uint16_t index, TIV* iv) : index_(index), vector_(iv) {}
    Iterator(const Iterator& other)
        : index_(other.index_), vector_(other.vector_) {}

    TT& operator*() const { return (*vector_)[index_]; }

    Iterator& operator=(const Iterator& other) {
      if (this != &other) {
        index_ = other.index_;
        vector_ = other.vector_;
      }
      return *this;
    }

    bool operator==(const Iterator& other) const {
      if (other.index_ != index_ || other.vector_ != vector_) {
        return false;
      }
      return true;
    }

    bool operator!=(const Iterator& other) const {
      return !this->operator==(other);
    }

    Iterator& operator++() {
      ++index_;
      return *this;
    }

    Iterator operator++(int) {
      Iterator tmp(*this);
      operator++();
      return tmp;
    }

    Iterator& operator--() {
      --index_;
      return *this;
    }

    Iterator operator--(int) {
      Iterator tmp(*this);
      operator--();
      return tmp;
    }

    Iterator& operator+=(const Iterator& rhs) {
      assert(vector_ == rhs.vector_);
      index_ += rhs.index_;
      return *this;
    }

    Iterator operator+(const Iterator& rhs) {
      assert(vector_ == rhs.vector_);
      Iterator tmp(*this);
      tmp.index_ += rhs.index_;
      return tmp;
    }

    Iterator& operator+=(uint16_t i) {
      index_ += i;
      return *this;
    }

    Iterator operator+(uint16_t i) {
      Iterator tmp(*this);
      tmp.index_ += i;
      return tmp;
    }

    Iterator& operator-=(const Iterator& rhs) {
      assert(vector_ == rhs.vector_);
      index_ -= rhs.index_;
      return *this;
    }

    Iterator operator-(const Iterator& rhs) {
      assert(vector_ == rhs.vector_);
      Iterator tmp(*this);
      tmp.index_ -= rhs.index_;
      return tmp;
    }

    Iterator& operator-=(uint16_t i) {
      index_ -= i;
      return *this;
    }

    Iterator operator-(uint16_t i) {
      Iterator tmp(*this);
      tmp.index_ -= i;
      return tmp;
    }
  };

  using value_type = T;
  using iterator = typename InlineVector::template Iterator<T, InlineVector>;
  using const_iterator =
      typename InlineVector::template Iterator<const T, const InlineVector>;

  explicit InlineVector(TRootIOCtor* io_ctor) {}  // Constructor for ROOT I/O
  InlineVector() {}

  InlineVector(const InlineVector<T, N>& other) {
    data_ = std::move(other.data_);
    size_ = other.size_;
    heap_capacity_ = other.heap_capacity_;
    if (other.heap_data_ != nullptr) {
      heap_data_ = new T[heap_capacity_];
      std::copy_n(other.heap_data_, HeapSize(), heap_data_);
    }
  }

  InlineVector(InlineVector<T, N>&& other) noexcept {
    data_ = other.data_;
    size_ = other.size_;
    heap_capacity_ = other.heap_capacity_;
    heap_data_ = other.heap_data_;
    other.heap_data_ = nullptr;
  }

  virtual ~InlineVector() {
    if (heap_data_ != nullptr) {
      heap_capacity_ = 0;
      delete[] heap_data_;
    }
  }

  /// Returns the number of elements that the container has currently
  /// allocated space for.
  uint16_t capacity() const { return N + heap_capacity_; }  // NOLINT

  /// Removes all elements from the container.
  /// Leaves capacity() unchanged.
  void clear() {  // NOLINT
    size_ = 0;
  }

  /// Increase the capacity of the container to a value that's greater or equal
  /// to new_capacity. If new_cap is greater than the current `capacity()`,
  /// new storage is allocated, otherwise the method does nothing.
  ///
  /// If new_cap is greater than `capacity()`, all iterators and references,
  /// including the past-the-end iterator, are invalidated.
  /// Otherwise, no iterators or references are invalidated.
  void reserve(uint16_t new_capacity) {  // NOLINT
    if (new_capacity > capacity()) {
      heap_capacity_ = new_capacity - N;
      T* new_heap_data = new T[heap_capacity_];
      if (heap_data_ != nullptr) {
        std::copy_n(heap_data_, HeapSize(), new_heap_data);
        delete[] heap_data_;
      }
      heap_data_ = new_heap_data;
    }
  }

  /// \brief returns the number of elements in this container
  uint16_t size() const { return size_; }  // NOLINT

  /// adds elements to this container and allocates additional memory on the
  /// heap if required
  void push_back(const T& element) {  // NOLINT
    if (size_ < N) {
      data_[size_++] = element;
    } else {
      // allocate heap memory
      assert(size_ != std::numeric_limits<uint16_t>::max() &&
             "Maxium number of elements exceeded");
      if (size_ == capacity()) {
        uint64_t tmp = static_cast<uint64_t>(capacity()) * kGrowFactor;
        uint16_t new_capacity = static_cast<uint16_t>(tmp);
        if (tmp > std::numeric_limits<uint16_t>::max()) {
          new_capacity = std::numeric_limits<uint16_t>::max();
        }
        reserve(new_capacity);
      }
      heap_data_[HeapSize()] = element;
      size_++;
    }
  }

  iterator erase(const iterator& it) {
    auto idx = it.index_;
    if (idx >= size_) {
      Log::Fatal(
          "InlineVector::erase",
          "You tried to erase an element that is outside the InlineVector.");
      return it;
    } else if (idx == size_ - 1) {
      // last element
      size_--;
      return end();
    }

    // element in the middle
    for (uint16_t i = it.index_; i < size_ - 1; i++) {
      (*this)[i] = (*this)[i + 1];
    }
    size_--;
    return iterator(idx, this);
  }

  std::vector<T> make_std_vector() const {  // NOLINT
    std::vector<T> std_vector(size_);
    for (uint16_t i = 0; i < size_; i++) {
      std_vector[i] = (*this)[i];
    }
    return std_vector;
  }

  InlineVector<T, N>& operator=(const InlineVector<T, N>& other) {
    if (this != &other) {
      data_ = other.data_;
      size_ = other.size_;
      heap_capacity_ = other.heap_capacity_;
      if (other.heap_data_ != nullptr) {
        if (heap_data_ != nullptr) {
          delete[] heap_data_;
        }
        heap_data_ = new T[heap_capacity_];
        if (size_ > N) {
          std::copy_n(other.heap_data_, HeapSize(), heap_data_);
        }
      }
    }
    return *this;
  }

  InlineVector<T, N>& operator=(InlineVector<T, N>&& other) {
    if (this != &other) {
      data_ = std::move(other.data_);
      size_ = other.size_;
      heap_capacity_ = other.heap_capacity_;
      heap_data_ = other.heap_data_;
      other.heap_data_ = nullptr;
      other.heap_capacity_ = 0;
    }
    return *this;
  }

  T& operator[](uint16_t index) {
    if (index < N) {
      return data_[index];
    } else {
      return heap_data_[index - N];
    }
  }

  const T& operator[](uint16_t index) const {
    if (index < N) {
      return data_[index];
    } else {
      return heap_data_[index - N];
    }
  }

  bool operator==(const InlineVector<T, N>& other) const {
    if (size_ != other.size_) {
      return false;
    }
    // inline data
    for (uint16_t i = 0; i < std::min(size_, N); i++) {
      if (data_[i] != other.data_[i]) {
        return false;
      }
    }
    // heap data
    if (size_ > N) {
      for (uint16_t i = 0; i < HeapSize(); i++) {
        if (heap_data_[i] != other.heap_data_[i]) {
          return false;
        }
      }
    }
    return true;
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const InlineVector<T, N>& other) {
    for (uint16_t i = 0; i < other.size_; i++) {
      out << other[i] << ", ";
    }
    return out;
  }

  iterator begin() { return iterator(0, this); }
  iterator end() { return iterator(size_, this); }
  const_iterator begin() const { return const_iterator(0, this); }
  const_iterator end() const { return const_iterator(size_, this); }

 private:
  static constexpr float kGrowFactor = 1.5;
  std::array<T, N> data_;
  uint16_t size_ = 0;
  UInt_t heap_capacity_ = 0;  // needed to help ROOT with array size
  T* heap_data_ = nullptr;    //[heap_capacity_]  // NOLINT

  uint16_t HeapSize() const {
    if (size_ < N) {
      return 0;
    }
    return size_ - N;
  }

  BDM_TEMPLATE_CLASS_DEF(InlineVector, 1);  // NOLINT
};

}  // namespace bdm

#endif  // CORE_CONTAINER_INLINE_VECTOR_H_
