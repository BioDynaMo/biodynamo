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

#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <algorithm>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "log.h"

#include "backend.h"
#include "root_util.h"
#include "simulation_object_util.h"
#include "type_util.h"

namespace bdm {

using std::enable_if;
using std::is_same;

template <typename TCompileTimeParam, typename TDerived>
class SimulationObject;

/// Contains implementation for SimulationObject that are specific to SOA
/// backend. The peculiarity of SOA objects is that it is simulation object
/// and container at the same time.
/// @see TransactionalVector
template <typename TCompileTimeParam, typename TDerived>
class SoaSimulationObject {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  template <typename, typename>
  friend class SoaSimulationObject;

  template <typename TTBackend>
  using MostDerived = typename TDerived::template type<
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;

  template <typename TBackend>
  using Self =
      SoaSimulationObject<typename TCompileTimeParam::template Self<TBackend>,
                          TDerived>;

  SoaSimulationObject() : to_be_removed_(), total_size_(1), size_(1) {}

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimulationObject(const Self<SoaRef> &other);

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimulationObject(const Self<Soa> &other);

  template <typename T>
  SoaSimulationObject(T *other, size_t idx)
      : kIdx(idx),
        mutex_(other->mutex_),
        to_be_removed_(other->to_be_removed_),
        total_size_(other->total_size_),
        size_(other->size_) {}

  virtual ~SoaSimulationObject() {}

  SoaSimulationObject &operator=(SoaSimulationObject &&other) {
    // mutex_ = std::move(other.mutex_);
    to_be_removed_ = std::move(other.to_be_removed_);
    total_size_ = other.total_size_;
    size_ = other.size_;
    return *this;
  }

  uint32_t GetElementIdx() const { return kIdx; }

  /// Returns the vector's size. Uncommited changes are not taken into account
  size_t size() const {  // NOLINT
    return size_;
  }

  /// Returns the number of elements in the container including non commited
  /// additions
  size_t TotalSize() const { return total_size_; }

  /// Thread safe version of std::vector::push_back
  template <typename TTBackend>
  void push_back(const MostDerived<TTBackend> &element) {  // NOLINT
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (total_size_ == size_) {
      PushBackImpl(element);
      size_++;
    } else {
      throw std::logic_error(
          "There are uncommited delayed additions to this container");
    }
  }

  /// Safe method to remove an element from this vector
  /// Does not invalidate, iterators, pointers or references.
  /// Changes do not take effect until they are commited.
  /// Upon commit removal has constant complexity @see Commit
  /// @param index remove element at the given index
  void DelayedRemove(size_t index) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    to_be_removed_.push_back(index);
  }

  /// Equivalent to std::vector<> clear - it removes all elements from all
  /// data members
  void clear() {  // NOLINT
    total_size_ = 0;
    size_ = 0;
    to_be_removed_.clear();
  }

  /// This method commits changes made by `DelayedPushBack` and `DelayedRemove`.
  /// CAUTION: \n
  ///   * Commit invalidates pointers and references returned by
  ///     `DelayedPushBack`. \n
  ///   * If memory reallocations are required all pointers or references
  ///     into this container are invalidated\n
  /// One removal has constant complexity. If the element which should be
  /// removed is not the last element it is swapped with the last one.
  /// (CAUTION: this invalidates pointers and references to the last element)
  /// In the next step it can be removed in constant time using pop_back. \n
  std::unordered_map<uint32_t, uint32_t> Commit() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::unordered_map<uint32_t, uint32_t> updated_indices;
    // commit delayed push backs
    size_ = total_size_;
    // commit delayed removes
    // sort indices in descending order to prevent out of bounds accesses
    auto descending = [](auto a, auto b) { return a > b; };
    std::sort(to_be_removed_.begin(), to_be_removed_.end(), descending);
    for (size_t idx : to_be_removed_) {
      assert(idx < size_ && "Removed index outside array boundaries");
      if (idx < size_ - 1) {
        SwapAndPopBack(idx, size_);
        updated_indices[size_ - 1] = idx;
      } else {
        PopBack();
      }
      size_--;
    }
    to_be_removed_.clear();

    // use implementation from TransactionalVector to avoid code duplication
    TransactionalVector<int>::ShortcutUpdatedIndices(&updated_indices);

    total_size_ = size_;
    return updated_indices;
  }

  /// Equivalent to std::vector<> reserve - it increases the capacity
  /// of all data member containers
  void reserve(size_t new_capacity) {}  // NOLINT

  template <typename Function>
  void ForEachDataMember(Function l) {}

  template <typename Function>
  void ForEachDataMemberIn(const std::set<std::string> &dm_selector,
                           Function f) {
    // validate data_members
    // all data members should have been removed from the set. Remaining
    // entries do not exist
    if (dm_selector.size() != 0) {
      std::stringstream sstr;
      for (auto &element : dm_selector) {
        sstr << element << ", ";
      }
      Fatal("ForEachDataMemberIn",
            "Please check your config file. The following data members do not "
            "exist: %s",
            sstr.str().c_str());
    }
  }

 protected:
  const size_t kIdx = 0;

  typename type_ternary_operator<is_same<Backend, SoaRef>::value,
                                 std::recursive_mutex &,
                                 std::recursive_mutex>::type mutex_;  //!

  /// vector of indices with elements which should be removed
  /// to_be_removed_ is of type vector<size_t>& if Backend == SoaRef;
  /// otherwise vector<size_t>
  typename type_ternary_operator<is_same<Backend, SoaRef>::value,
                                 std::vector<size_t> &,
                                 std::vector<size_t>>::type to_be_removed_;

  /// Append a scalar element
  virtual void PushBackImpl(const MostDerived<Scalar> &other) { total_size_++; }

  /// Append a soa ref element
  virtual void PushBackImpl(const MostDerived<SoaRef> &other) { total_size_++; }

  /// Swap element with last element if and remove last element
  virtual void SwapAndPopBack(size_t index, size_t size) {}

  /// Remove last element
  virtual void PopBack() {}

 private:
  /// vector of indices with elements which should be removed
  /// to_be_removed_ is of type vector<size_t>& if Backend == SoaRef;
  /// otherwise vector<size_t>
  typename type_ternary_operator<is_same<Backend, SoaRef>::value, size_t &,
                                 size_t>::type total_size_ = 0;

  /// size_ is of type size_t& if Backend == SoaRef; otherwise size_t
  typename type_ternary_operator<is_same<Backend, SoaRef>::value, size_t &,
                                 size_t>::type size_;

  // use modified class def, due to possible SoaRef backend
  BDM_ROOT_CLASS_DEF(SoaSimulationObject, 1);
};

/// Contains implementations for SimulationObject that are specific to scalar
/// backend
template <typename TCompileTimeParam, typename TDerived>
class ScalarSimulationObject {
 public:
  template <typename TTBackend>
  using MostDerived = typename TDerived::template type<
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;

  ScalarSimulationObject() : element_idx_(0) {}
  ScalarSimulationObject(const ScalarSimulationObject &other)
      : element_idx_(other.element_idx_) {}

  ScalarSimulationObject &operator=(ScalarSimulationObject &&other) {
    element_idx_ = other.element_idx_;
    return *this;
  }

  virtual ~ScalarSimulationObject() {}

  std::size_t size() const { return 1; }  // NOLINT

  uint32_t GetElementIdx() const { return element_idx_; }

  // assign the array index of this object in the ResourceManager
  void SetElementIdx(uint32_t element_idx) { element_idx_ = element_idx; }

 protected:
  static const std::size_t kIdx = 0;
  // array index of this object in the ResourceManager
  uint32_t element_idx_ = 0;

  /// Append a scalar element
  virtual void PushBackImpl(const MostDerived<Scalar> &other) {}

  /// Append a SoaRef element
  virtual void PushBackImpl(const MostDerived<SoaRef> &other) {}

  /// Swap element with last element if and remove last element
  virtual void SwapAndPopBack(size_t index, size_t size) {}

  /// Remove last element
  virtual void PopBack() {}

  BDM_ROOT_CLASS_DEF(ScalarSimulationObject, 2);
};

/// Helper type trait to map backends to simulation object implementations
template <typename TCompileTimeParam, typename TDerived>
struct SimulationObjectImpl {
  using Backend = typename TCompileTimeParam::Backend;
  using type = typename type_ternary_operator<
      is_same<Backend, Scalar>::value,
      ScalarSimulationObject<TCompileTimeParam, TDerived>,
      SoaSimulationObject<TCompileTimeParam, TDerived>>::type;
};

/// Contains code required by all simulation objects
template <typename TCompileTimeParam, typename TDerived>
class SimulationObject
    : public SimulationObjectImpl<TCompileTimeParam, TDerived>::type {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  using Base = typename SimulationObjectImpl<TCompileTimeParam, TDerived>::type;
  template <typename TTBackend>
  using MostDerived = typename TDerived::template type<
      typename TCompileTimeParam::template Self<TTBackend>, TDerived>;

  template <typename, typename>
  friend class SimulationObject;

  SimulationObject() : Base() {}
  SimulationObject(const SimulationObject &) = default;

  template <typename T>
  SimulationObject(T *other, size_t idx) : Base(other, idx) {}

  virtual ~SimulationObject() {}

  /// Used internally to create the same object, but with
  /// different backend - required since inheritance chain is not known
  /// inside a mixin.
  template <typename TTBackend>
  using Self =
      SimulationObject<typename TCompileTimeParam::template Self<TTBackend>,
                       TDerived>;

  /// This function determines if the type of this simulation object is the same
  /// as `TSo` without taking the backend into account.
  /// @tparam TSo Simulation object type with any backend
  template <typename TSo>
  static constexpr bool IsSoType() {
    using TSoScalar = typename TSo::template Self<Scalar>;
    return std::is_same<MostDerived<Scalar>, TSoScalar>::value;
  }

  /// This function determines if the type of this simulation object is the same
  /// as `object` without taking the backend into account.
  /// @param object simulation object can have any backend
  template <typename TSo>
  static constexpr bool IsSoType(const TSo *object) {
    using Type = std::decay_t<std::remove_pointer_t<decltype(object)>>;
    using ScalarType = typename Type::template Self<Scalar>;
    return std::is_same<MostDerived<Scalar>, ScalarType>::value;
  }

  /// Casts this to a simulation object of type `TSo` with the current `Backend`
  /// This function is used to simulate if constexpr functionality and won't be
  /// needed after we swith to C++17
  /// @tparam TSo target simulaton object type with any backend
  template <typename TSo>
  constexpr auto &&ReinterpretCast() {
    using TargetType = typename TSo::template Self<Backend>;
    return reinterpret_cast<TargetType &&>(*this);
  }

  /// Empty default implementation to update references of simulation objects
  /// that changed its memory position.
  /// @param update_info vector index = type_id, map stores (old_index ->
  /// new_index)
  void UpdateReferences(
      const std::vector<std::unordered_map<uint32_t, uint32_t>> &update_info) {}

  /// Implementation to update a single reference if `reference.GetElementIdx()`
  /// is a key in `updates`.
  /// @tparam TReference type of the reference. Must have a `GetElementIdx` and
  ///         `SetElementIdx` method.
  /// @param reference reference whos `element_idx` will be updated
  /// @param updates map that contains the update information
  ///        (old_index -> new_index) for a specific simulation object type
  template <typename TReference>
  void UpdateReference(TReference *reference,
                       const std::unordered_map<uint32_t, uint32_t> &updates) {
    auto search = updates.find(reference->GetElementIdx());
    if (search != updates.end()) {
      reference->SetElementIdx(search->second);
    }
  }

  Self<Backend> &operator=(const Self<Scalar> &) { return *this; }

  SimulationObject &operator=(SimulationObject &&other) {
    Base::operator=(std::move(other));
    return *this;
  }

  MostDerived<Backend> *operator->() {
    return static_cast<MostDerived<Backend> *>(this);
  }

  const MostDerived<Backend> *operator->() const {
    return static_cast<const MostDerived<Backend> *>(this);
  }

  BDM_ROOT_CLASS_DEF_OVERRIDE(SimulationObject, 1);
};

/// type alias to be consistent with naming convention for simulation object
/// extension
/// \see BDM_SIM_OBJECT
template <typename TCompileTimeParam, typename TDerived>
using SimulationObject_TCTParam_TDerived =
    SimulationObject<TCompileTimeParam, TDerived>;

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
