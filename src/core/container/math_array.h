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

#ifndef CORE_CONTAINER_MATH_ARRAY_
#define CORE_CONTAINER_MATH_ARRAY_

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace bdm {

/// Array with a fixed number of elements. It implements the same behaviour
/// of the standard `MathArray<T, N>` container. However, it provides also
/// several custom mathematical operations (e.g. Sum(), Norm() etc.).
template <class T, std::size_t N>
class MathArray {  // NOLINT
 public:
  constexpr MathArray() : data_() {
	  /// std::fill will become constexpr with C++20
	  for (size_t i=0; i<N; i++)
	  {
		  data_[i] = 0;
	  }
  }
  constexpr MathArray(std::initializer_list<T> l) : MathArray<T, N>() {
    assert(l.size() == N);
	for (size_t i=0; i<N; i++)
	{
	  data_[i] = *(l.begin()+i);
	}
  }

  inline const T* data() const { return &data_[0]; }  // NOLINT

  inline const size_t size() const { return N; }  // NOLINT

  inline const bool empty() const { return N == 0; }  // NOLINT

  T& operator[](size_t idx) { return data_[idx]; }

  const T& operator[](size_t idx) const { return data_[idx]; }

  T& at(size_t idx) {  // NOLINT
    if (idx > size() || idx < 0) {
      throw std::out_of_range("The index is out of range");
    }
    return data_[idx];
  }

  const T* begin() const { return &(data_[0]); }  // NOLINT

  const T* end() const { return &(data_[N]); }  // NOLINT

  T* begin() { return &(data_[0]); }  // NOLINT

  T* end() { return &(data_[N]); }  // NOLINT

  T& front() { return *(this->begin()); }  // NOLINT

  T& back() {  // NOLINT
    auto tmp = this->end();
    tmp--;
    return *tmp;
  }

  MathArray& operator=(const MathArray& other) {
    if (this != &other) {
      assert(other.size() == N);
      std::copy(other.data_, other.data_ + other.size(), data_);
    }
    return *this;
  }

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
    MathArray<T, N> tmp(*this);
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
    MathArray<T, N> tmp(*this);
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
    MathArray tmp(*this);
    tmp += rhs;
    return tmp;
  }

  MathArray& operator+=(const T& rhs) {
    assert(N == rhs.size());
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] += rhs;
    }
    return *this;
  }

  MathArray operator+(const T& rhs) {
    assert(size() == rhs.size());
    MathArray tmp(*this);
    tmp += rhs;
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
    MathArray tmp(*this);
    tmp -= rhs;
    return tmp;
  }

  MathArray& operator-=(const T& rhs) {
    assert(size() == rhs.size());
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] -= rhs;
    }
    return *this;
  }

  MathArray operator-(const T& rhs) {
    assert(size() == rhs.size());
    MathArray tmp(*this);
    tmp -= rhs;
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

  MathArray& operator*=(const T& k) {
#pragma omp simd

    for (size_t i = 0; i < N; i++) {
      data_[i] *= k;
    }
    return *this;
  }

  MathArray operator*(const T& k) {
    MathArray tmp(*this);
    tmp *= k;
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
    MathArray tmp(*this);
    tmp /= k;
    return tmp;
  }

  MathArray& fill(const T& k) {  // NOLINT
	std::fill(std::begin(data_), std::end(data_), k);
    return *this;
  }

  T Sum() const { return std::accumulate(begin(), end(), 0); }

  T Norm() const {
    T result = 0;
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      result += data_[i] * data_[i];
    }
    result = std::sqrt(result);

    return result == 0 ? 1.0 : result;
  }

  MathArray& Normalize() {
    T norm = Norm();
#pragma omp simd
    for (size_t i = 0; i < N; i++) {
      data_[i] /= norm;
    }
    return *this;
  }

 private:
  T data_[N];
};

using Double3 = MathArray<double, 3>;

}  // namespace bdm

#endif  // CORE_CONTAINER_MATH_ARRAY_H_
