#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <algorithm>
#include <mutex>
#include <type_traits>
#include <vector>

#include "backend.h"
#include "root_util.h"
#include "type_util.h"

namespace bdm {

using std::enable_if;
using std::is_same;

template <typename TCompileTimeParam>
class SimulationObject;

/// Contains implementation for SimulationObject that are specific to SOA
/// backend. The peculiarity of SOA objects is that it is simulation object
/// and container at the same time.
/// @see TransactionalVector
template <typename TCompileTimeParam>
class SoaSimulationObject {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  template <typename T>
  friend class SoaSimulationObject;

  template <typename TBackend>
  using Self =
      SoaSimulationObject<typename TCompileTimeParam::template Self<TBackend>>;

  SoaSimulationObject() : to_be_removed_(), size_(1) {}

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
        size_(other->size_) {}

  virtual ~SoaSimulationObject() {}

  /// Returns the vector's size. Uncommited changes are not taken into account
  size_t size() const {  // NOLINT
    return size_;
  }

  /// Thread safe version of std::vector::push_back
  void push_back(
      const SimulationObject<typename TCompileTimeParam::template Self<Scalar>>
          &element) {  // NOLINT
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
  virtual void PushBackImpl(
      const SimulationObject<typename TCompileTimeParam::template Self<Scalar>>
          &other) {
    size_++;
  }

  /// Swap element with last element if and remove last element
  virtual void SwapAndPopBack(size_t index, size_t size) { size_--; }

  /// Remove last element
  virtual void PopBack(size_t index, size_t size) { size_--; }

 private:
  /// size_ is of type size_t& if Backend == SoaRef; otherwise size_t
  typename type_ternary_operator<is_same<Backend, SoaRef>::value, size_t &,
                                 size_t>::type size_;

  // use modified class def, due to possible SoaRef backend
  BDM_ROOT_CLASS_DEF(SoaSimulationObject, 1);
};

/// Contains implementations for SimulationObject that are specific to scalar
/// backend
template <typename TCompileTimeParam>
class ScalarSimulationObject {
 public:
  virtual ~ScalarSimulationObject() {}

  std::size_t size() const { return 1; }  // NOLINT

 protected:
  static const std::size_t kIdx = 0;

  /// Append a scalar element
  virtual void PushBackImpl(
      const SimulationObject<typename TCompileTimeParam::template Self<Scalar>>
          &other) {}

  /// Swap element with last element if and remove last element
  virtual void SwapAndPopBack(size_t index, size_t size) {}

  /// Remove last element
  virtual void PopBack(size_t index, size_t size) {}

  ClassDef(ScalarSimulationObject, 1);
};

/// Helper type trait to map backends to simulation object implementations
template <typename TCompileTimeParam>
struct SimulationObjectImpl {
  using Backend = typename TCompileTimeParam::Backend;
  using type = typename type_ternary_operator<
      is_same<Backend, Scalar>::value,
      ScalarSimulationObject<TCompileTimeParam>,
      SoaSimulationObject<TCompileTimeParam>>::type;
};

/// Contains code required by all simulation objects
template <typename TCompileTimeParam>
class SimulationObject : public SimulationObjectImpl<TCompileTimeParam>::type {
 public:
  using Backend = typename TCompileTimeParam::Backend;
  using Base = typename SimulationObjectImpl<TCompileTimeParam>::type;

  template <typename T>
  friend class SimulationObject;

  SimulationObject() : Base() {}

  template <typename T>
  SimulationObject(T *other, size_t idx) : Base(other, idx) {}

  virtual ~SimulationObject() {}

  /// Used internally to create the same object, but with
  /// different backend - required since inheritance chain is not known
  /// inside a mixin.
  template <typename TTBackend>
  using Self =
      SimulationObject<typename TCompileTimeParam::template Self<TTBackend>>;

  SimulationObject<TCompileTimeParam> &operator=(const Self<Scalar> &) {
    return *this;
  }

  BDM_ROOT_CLASS_DEF_OVERRIDE(SimulationObject, 1);
};

/// type alias to be consistent with naming convention for simulation object
/// extension
/// \see BDM_SIM_OBJECT
template <typename TCompileTimeParam>
using SimulationObjectT = SimulationObject<TCompileTimeParam>;

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
