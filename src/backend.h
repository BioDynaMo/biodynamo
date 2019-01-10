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

#ifndef BACKEND_H_
#define BACKEND_H_

#include <exception>
#include <memory>
#include <utility>
#include <vector>

#include "macros.h"
#include "root_util.h"

namespace bdm {

/// VectorPlaceholder has the same interface as a std::vector
/// but no data members. Therefore, there is no dynamic memory
/// allocation and the size of an instance is almost zero.
/// Use case: In a templated class a std::vector data member is
/// not used for all template parameters.
template <typename T>
class VectorPlaceholder {
 public:
  using value_type = T;
  VectorPlaceholder() {}
  VectorPlaceholder(std::initializer_list<T> list) {}

  std::size_t size() const { return 0; }  // NOLINT

  T& operator[](const size_t idx) {
    throw std::logic_error("Unsupported operation!");
  }

  const T& operator[](const size_t idx) const {
    throw std::logic_error("Unsupported operation!");
  }

  void push_back(const T& other) {  // NOLINT
    throw std::logic_error("Unsupported operation!");
  }
  void emplace_back(const T& other) {  // NOLINT
    throw std::logic_error("Unsupported operation!");
  }

  void pop_back() {  // NOLINT
    throw std::logic_error("Unsupported operation!");
  }

  T* begin() { throw std::logic_error("Unsupported operation!"); }  // NOLINT
  T* end() { throw std::logic_error("Unsupported operation!"); }    // NOLINT

  const T* begin() const {  // NOLINT
    throw std::logic_error("Unsupported operation!");
  }
  const T* end() const {  // NOLINT
    throw std::logic_error("Unsupported operation!");
  }
};

/// \brief This class represents an array with exactly one element
///
/// Needed for AOSOA: Objects will store a single e.g. real_v instead of N
/// instances. However code was written for SOA and expects an array interface
/// which is exposed with this class.
/// Makes it easy for the compiler to optimize out the extra call to operator[]
/// Didn't work with std::array<T, 1>
template <typename T>
class OneElementArray {
 public:
  using value_type = T;
  OneElementArray() : data_() {}
  explicit OneElementArray(TRootIOCtor* io_ctor) {}  // Constructor for ROOT I/O
  explicit OneElementArray(const T& data) : data_(data) {}
  explicit OneElementArray(T&& data) : data_(data) {}
  OneElementArray(std::initializer_list<T> list) : data_(*list.begin()) {}
  virtual ~OneElementArray() {}

  std::size_t size() const { return 1; }  // NOLINT

  BDM_FORCE_INLINE T& operator[](const size_t idx) { return data_; }

  BDM_FORCE_INLINE const T& operator[](const size_t idx) const { return data_; }

  /// This function is only used to allow compilation of SOA simulation objects,
  /// but must not be called!
  /// Some methods required for SOA simulation objects need to be virtual.
  /// Virtualization and templating cannot be used at the same time.
  /// Consequently, conditional compilation of those methods only for the SOA
  /// memory layout is not feasable. This means that operations in these
  /// functions must also be supported for Scalar backends. Otherwise,
  /// compilation would fail.
  void push_back(const T& other) {  // NOLINT
    throw std::logic_error("Unsupported operation!");
  }

  /// This function is only used to allow compilation of SOA simulation objects,
  /// but must not be called!
  /// Some methods required for SOA simulation objects need to be virtual.
  /// Virtualization and templating cannot be used at the same time.
  /// Consequently, conditional compilation of those methods only for the SOA
  /// memory layout is not feasable. This means that operations in these
  /// functions must also be supported for Scalar backends. Otherwise,
  /// compilation would fail.
  void pop_back() {  // NOLINT
    throw std::logic_error("Unsupported operation!");
  }

  T* begin() { return &data_; }    // NOLINT
  T* end() { return &data_ + 1; }  // NOLINT

  const T* begin() const { return &data_; }    // NOLINT
  const T* end() const { return &data_ + 1; }  // NOLINT

 private:
  T data_;
  BDM_ROOT_CLASS_DEF(OneElementArray, 1)  // NOLINT
};

struct Scalar {
  /// Data type used to store data members of a class
  template <typename T>
  using vec = OneElementArray<T>;  // NOLINT

  /// Data type to store a collection of simulation objects with this backend
  template <typename T>
  using Container = std::vector<T>;
};

struct Soa {
  /// Data type used to store data members of a class
  template <typename T>
  using vec = std::vector<T>;  // NOLINT

  /// Data type to store a collection of simulation objects with this backend
  template <typename T>
  using Container = T;
};

struct SoaRef {
  /// Data type used to store data members of a class
  template <typename T>
  using vec = typename Soa::template vec<T>&;  // NOLINT

  /// Data type to store a collection of simulation objects with this backend
  template <typename T>
  using Container = T;
};

}  // namespace bdm

#endif  // BACKEND_H_
