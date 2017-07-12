#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <algorithm>
#include <mutex>
#include <type_traits>
#include <vector>
#include "backend.h"
#include "type_util.h"

namespace bdm {

using std::enable_if;
using std::is_same;

template <typename>
class SimulationObject;

/// Contains implementation for SimulationObject that are specific to SOA
/// backend. The peculiarity of SOA objects is that it is simulation object
/// and container at the same time.
/// @see TransactionalVector
template <typename TBackend>
class SoaSimulationObject {
 public:
  template <typename T>
  friend class SoaSimulationObject;

  SoaSimulationObject() : to_be_removed_(), size_(1) {}

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimulationObject(const SoaSimulationObject<SoaRef>& other);

  /// Detect failing return value optimization (RVO)
  /// Copy-ctor declaration to please compiler, but missing implementation.
  /// Therefore, if it gets called somewhere (failing RVO optimization),
  /// the linker would throw an error.
  explicit SoaSimulationObject(const SoaSimulationObject<Soa>& other);

  template <typename T>
  SoaSimulationObject(T* other, size_t idx)
      : kIdx(idx),
        mutex_(other->mutex_),
        to_be_removed_(other->to_be_removed_),
        size_(other->size_) {}

  virtual ~SoaSimulationObject() {}

  /// Returns the vector's size. Uncommited changes are not taken into account
  size_t size() const {  // NOLINT
    return size_;
  }

  /// Returns the object's id.
  size_t id() const {  // NOLINT
    return kIdx;
  }

  /// Thread safe version of std::vector::push_back
  void push_back(const SimulationObject<Scalar>& element) {  // NOLINT
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    PushBackImpl(element);
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
  void clear() { size_ = 0; }  // NOLINT

  /// Equivalent to std::vector<> reserve - it increases the capacity
  /// of all data member containers
  void reserve(size_t new_capacity) {}  // NOLINT

 protected:
  const size_t kIdx = 0;

  typename type_ternary_operator<is_same<TBackend, SoaRef>::value,
                                 std::recursive_mutex&,
                                 std::recursive_mutex>::type mutex_;

  /// vector of indices with elements which should be removed
  /// to_be_removed_ is of type vector<size_t>& if TBackend == SoaRef;
  /// otherwise vector<size_t>
  typename type_ternary_operator<is_same<TBackend, SoaRef>::value,
                                 std::vector<size_t>&,
                                 std::vector<size_t>>::type to_be_removed_;

  /// Append a scalar element
  virtual void PushBackImpl(const SimulationObject<Scalar>& other) { size_++; }

  /// Swap element with last element if and remove last element
  virtual void SwapAndPopBack(size_t index, size_t size) { size_--; }

  /// Remove last element
  virtual void PopBack(size_t index, size_t size) { size_--; }

 private:
  /// size_ is of type size_t& if TBackend == SoaRef; otherwise size_t
  typename type_ternary_operator<is_same<TBackend, SoaRef>::value, size_t&,
                                 size_t>::type size_;
};

/// Contains implementations for SimulationObject that are specific to scalar
/// backend
class ScalarSimulationObject {
 public:
  virtual ~ScalarSimulationObject() {}

  std::size_t size() const { return 1; }  // NOLINT

  /// Returns the object's id.
  size_t id() const {  // NOLINT
    return kIdx;
  }

 protected:
  static const std::size_t kIdx = 0;

  /// Append a scalar element
  virtual void PushBackImpl(const SimulationObject<Scalar>& other) {}

  /// Swap element with last element if and remove last element
  virtual void SwapAndPopBack(size_t index, size_t size) {}

  /// Remove last element
  virtual void PopBack(size_t index, size_t size) {}
};

/// Helper type trait to map backends to simulation object implementations
template <typename TBackend>
struct SimulationObjectImpl {};

template <>
struct SimulationObjectImpl<Soa> {
  typedef SoaSimulationObject<Soa> type;  // NOLINT
};

template <>
struct SimulationObjectImpl<SoaRef> {
  typedef SoaSimulationObject<SoaRef> type;  // NOLINT
};

template <>
struct SimulationObjectImpl<Scalar> {
  typedef ScalarSimulationObject type;  // NOLINT
};

/// Contains code required by all simulation objects
template <typename TBackend = Scalar>
class SimulationObject : public SimulationObjectImpl<TBackend>::type {
 public:
  using Base = typename SimulationObjectImpl<TBackend>::type;
  using Backend = TBackend;

  template <typename T>
  friend class SimulationObject;

  SimulationObject() : Base() {}

  template <typename T>
  SimulationObject(T* other, size_t idx) : Base(other, idx) {}

  virtual ~SimulationObject() {}

  SimulationObject<Backend>& operator=(const SimulationObject<Scalar>&) {
    return *this;
  }

  /// Used internally to create the same object, but with
  /// different backend - required since inheritance chain is not known
  /// inside a mixin.
  template <typename TTBackend>
  using Self = SimulationObject<TTBackend>;
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
