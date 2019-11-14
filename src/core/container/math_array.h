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

#ifndef CORE_CONTAINER_MATH_ARRAY_H_
#define CORE_CONTAINER_MATH_ARRAY_H_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "core/util/root.h"

namespace bdm {

/// Array with a fixed number of elements. It implements the same behaviour
/// of the standard `std::array<T, N>` container. However, it provides also
/// several custom mathematical operations (e.g. Sum(), Norm() etc.).
template <class T, std::size_t N>
class MathArray {  // NOLINT
 public:
  /// Default constructor
  constexpr MathArray() {}

  /// Constructor which accepts an std::initiliazer_list to set
  /// the array's content.
  /// \param l an initializer list
  constexpr MathArray(std::initializer_list<T> l) {
    assert(l.size() == N);
    auto it = l.begin();
    for (uint64_t i = 0; i < N; i++) {
      data_[i] = *(it++);
    }
  }

  /// Return a pointer to the underlying data.
  /// \return cont T pointer to the first entry of the array.
  inline const T* data() const { return &data_[0]; }  // NOLINT

  /// Return the size of the array.
  /// \return integer denoting the array's size.
  inline const size_t size() const { return N; }  // NOLINT

  /// Check if the array is empty.
  /// \return true if size() == 0, false otherwise.
  inline const bool empty() const { return N == 0; }  // NOLINT

  /// Overloaded array subscript operator. It does not perform
  /// any boundary checks.
  /// \param idx element's index.
  /// \return the requested element.
  T& operator[](size_t idx) { return data_[idx]; }

  /// Const overloaded array subscript operator.
  /// \param idx element's index.
  /// \return the requested element.
  const T& operator[](size_t idx) const { return data_[idx]; }

  /// Returns the element at the given position. It will throw
  /// an std::out_of_range exception if the given index is out
  /// of the array's boundaries.
  /// \param idx the index of the element.
  /// \return the requested element.
  T& at(size_t idx) noexcept(false) {  // NOLINT
    if (idx > size() || idx < 0) {
      throw std::out_of_range("The index is out of range");
    }
    return data_[idx];
  }

  const T* begin() const { return &(data_[0]); }  // NOLINT

  const T* end() const { return &(data_[N]); }  // NOLINT

  T* begin() { return &(data_[0]); }  // NOLINT

  T* end() { return &(data_[N]); }  // NOLINT

  /// Returns the element at the beginning of the array.
  /// \return first element.
  T& front() { return *(this->begin()); }  // NOLINT

  /// Return the element at the end of the array.
  /// \return last element.
  T& back() {  // NOLINT
    auto tmp = this->end();
    tmp--;
    return *tmp;
  }

  /// Assignment operator.
  /// \param other the other MathArray instance.
  /// \return the current MathArray.
  MathArray& operator=(const MathArray& other) {
    if (this != &other) {
      assert(other.size() == N);
      std::copy(other.data_, other.data_ + other.size(), data_);
    }
    return *this;
  }

  /// Equality operator.
  /// \param other a MathArray instance.
  /// \return true if they have the same content, false otherwise.
  bool operator==(const MathArray& other) const {
    if (other.size() != N) {
      return false;
    }
    for (size_t i = 0; i < N; i++) {
      if (other[i] != data_[i]) {
        return false;
      }
    }
    return true;
  }

  MathArray& operator++() {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      ++data_[i];
    }
    return *this;
  }

  MathArray operator++(int) {
    MathArray tmp(*this);
    operator++();
    return tmp;
  }

  MathArray& operator--() {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      --data_[i];
    }
    return *this;
  }

  MathArray operator--(int) {
    MathArray tmp(*this);
    operator--();
    return tmp;
  }

  MathArray& operator+=(const MathArray& rhs) {
    assert(N == rhs.size());
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] += rhs[i];
    }
    return *this;
  }

  MathArray operator+(const MathArray& rhs) {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] + rhs[i];
    }
    return tmp;
  }

  const MathArray operator+(const MathArray& rhs) const {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] + rhs[i];
    }
    return tmp;
  }

  MathArray& operator+=(const T& rhs) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] += rhs;
    }
    return *this;
  }

  MathArray operator+(const T& rhs) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] + rhs;
    }
    return tmp;
  }

  MathArray& operator-=(const MathArray& rhs) {
    assert(size() == rhs.size());
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] -= rhs[i];
    }
    return *this;
  }

  MathArray operator-(const MathArray& rhs) {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] - rhs[i];
    }
    return tmp;
  }

  const MathArray operator-(const MathArray& rhs) const {
    assert(size() == rhs.size());
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] - rhs[i];
    }
    return tmp;
  }

  MathArray& operator-=(const T& rhs) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] -= rhs;
    }
    return *this;
  }

  MathArray operator-(const T& rhs) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] - rhs;
    }
    return tmp;
  }

  T& operator*=(const MathArray& rhs) = delete;

  T operator*(const MathArray& rhs) {
    assert(size() == rhs.size());
    T result = 0;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      result += data_[i] * rhs[i];
    }
    return result;
  }

  const T operator*(const MathArray& rhs) const {
    assert(size() == rhs.size());
    T result = 0;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      result += data_[i] * rhs[i];
    }
    return result;
  }

  MathArray& operator*=(const T& k) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] *= k;
    }
    return *this;
  }

  MathArray operator*(const T& k) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] * k;
    }
    return tmp;
  }

  const MathArray operator*(const T& k) const {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] * k;
    }
    return tmp;
  }

  MathArray& operator/=(const T& k) {
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] /= k;
    }
    return *this;
  }

  MathArray operator/(const T& k) {
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      tmp[i] = data_[i] / k;
    }
    return tmp;
  }

  /// Fill the MathArray with a constant value.
  /// \param k the constant value
  /// \return the array
  MathArray& fill(const T& k) {  // NOLINT
    std::fill(std::begin(data_), std::end(data_), k);
    return *this;
  }

  /// Return the sum of all the array's elements.
  /// \return sum of the array's content.
  T Sum() const { return std::accumulate(begin(), end(), 0); }

  /// Compute the norm of the array's content.
  /// \return array's norm.
  T Norm() const {
    T result = 0;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      result += data_[i] * data_[i];
    }
    result = std::sqrt(result);

    return result == 0 ? 1.0 : result;
  }

  /// Normalize the array. It will be done in-place.
  /// \return the normalized array.
  MathArray& Normalize() {
    T norm = Norm();
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] /= norm;
    }
    return *this;
  }

  /// Compute the entry wise product given another array
  /// of the same size.
  /// \param rhs the other array
  /// \return a new array with the result
  MathArray EntryWiseProduct(const MathArray& rhs) {
    assert(rhs.size() == N);
    MathArray tmp;
#pragma omp simd
    for (size_t i = 0; i < N; ++i) {
      tmp[i] = data_[i] * rhs[i];
    }
    return tmp;
  }

 private:
  T data_[N];
  BDM_CLASS_DEF_NV(MathArray, 1);  // NOLINT
};

template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& o, const MathArray<T, N>& arr) {
  for (size_t i = 0; i < N; i++) {
    o << arr[i];
    if (i != N - 1) {
      o << ", ";
    }
  }
  return o;
}

/// Alias for a size 3 MathArray
using Double3 = MathArray<double, 3>;

/// Alias for a size 4 MathArray
using Double4 = MathArray<double, 4>;

}  // namespace bdm

#endif  // CORE_CONTAINER_MATH_ARRAY_H_
