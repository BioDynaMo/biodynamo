#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <type_traits>

#include "aosoa.h"
#include "backend.h"
#include "inline_vector.h"
#include "simulation_object_util.h"
#include "type_util.h"

using std::enable_if;
using std::is_same;

namespace bdm {

/// default data member selector which does not remove any data members
/// belongs to multiform_object.h, but to avoid circular reference declaration
/// has been moved to this source file
template <typename Type, typename EnclosingClass, int id>
struct SelectAllMembers {
  typedef Type type;
};

/// Contains implementations for SimulationObject that are specific to scalar
/// backends
class ScalarSimulationObject {
 public:
  virtual ~ScalarSimulationObject() {}

  std::size_t size() const { return 1; }

  bool is_full() const { return true; }

 protected:
  static const std::size_t idx_ = 0;
};

/// Contains implementations for SimulationObject that are specific to vector
/// backends
class VectorSimulationObject {
 public:
  using Backend = VcVectorBackend;

  virtual ~VectorSimulationObject() {}

  /// Set the number of scalar elements inside this vector
  void SetSize(std::size_t size) { size_ = size; }

  /// Returns number of scalar element inside this vector
  std::size_t size() const { return size_; }

  /// This function returns the number of scalar elements in this container
  /// container. In this context the same as size() and ElementsCurrentVector()
  /// but for other Backends they change their semantic
  std::size_t Elements() const { return size_; }

  /// \brief Returns the number of scalar elements in the current vector.
  /// In this context the same as size() and ElementsCurrentVector()
  /// but for other Backends they change their semantic.
  /// Required to have a consistent way to obtain this information between
  /// vector and soa backends
  std::size_t ElementsCurrentVector() const { return size_; }

  /// Returns the number of vectors of this object which is allways one
  std::size_t Vectors() const { return 1; }

  /// Returns true if this vector is fully filled with scalar elements
  /// false otherwise
  bool is_full() const { return size_ == Backend::kVecLen; }

  /// Append a scalar simulation object to this vector
  /// Copying of data members must be done in subclass.
  /// see BDM_CLASS_HEADER macro
  void push_back(const ScalarSimulationObject& other) { size_++; }

 protected:
  static const std::size_t idx_ = 0;
  /// Number of scalar element inside this vector
  std::size_t size_ = Backend::kVecLen;
};

/// Contains implementations for SimulationObject that are specific to soa
/// backends
template <typename TBackend>
class SoaSimulationObject {
 public:
  using Backend = TBackend;
  template <typename Type, typename EnclosingClass, int id>
  using MemberSelector = SelectAllMembers<Type, EnclosingClass, id>;

  template <typename T>
  friend class SoaSimulationObject;

  template <typename T = Backend>
  SoaSimulationObject(
      typename enable_if<is_same<T, VcSoaBackend>::value>::type* = 0)
      : size_(1), size_last_vector_(VcVectorBackend::kVecLen) {}

  /// Constructor to create SoaRefBackend.
  /// Only compiled if T == VcSoaRefBackend
  /// Template parameter required for enable_if
  template <typename T = Backend>
  SoaSimulationObject(
      SoaSimulationObject<VcSoaBackend>* other,
      typename enable_if<is_same<T, VcSoaRefBackend>::value>::type* = 0)
      : size_(other->size_), size_last_vector_(other->size_last_vector_) {}

  virtual ~SoaSimulationObject() {}

  /// Returns the number of vector elements in this object
  size_t size() const { return size_; }

  /// Returns true if the last vector does not have free scalar slots
  bool is_full() const { return size_last_vector_ == VcVectorBackend::kVecLen; }

  /// Appends a vector element
  void push_back(const VectorSimulationObject& other) {  // FIXME
    size_++;
    size_last_vector_ = other.ElementsCurrentVector();
  }

  /// Equivalent to std::vector<> clear - it removes all elements from all
  /// data members
  void clear() {
    size_ = 0;
    size_last_vector_ = 0;
  }

  // equivalent to std::vector<> reserve - it increases the capacity
  // of all data member containers
  void reserve(std::size_t new_capacity) {}

  /// Returns the number of vector elements in this object
  std::size_t Vectors() const { return size(); }

  /// This function returns the number of scalar simulation objects in this
  /// container. Assumes that only the last vector may not be fully
  /// initialized.
  std::size_t Elements() const {
    if (Vectors() != 0) {
      return (Vectors() - 1) * Backend::kVecLen + size_last_vector_;
    } else {
      return 0;
    }
  }

  /// This function returns the number of scalar simulation objects in the
  /// vector sepecified by idx_. Assumes that only the last vector may not
  /// be full.
  std::size_t ElementsCurrentVector() const {
    return idx_ == size_ - 1 ? size_last_vector_ : VcVectorBackend::kVecLen;
  }

  /// Adds a scalar element at the back of this collection.
  /// If the last vector is not full, the scalar instance will
  /// be copied into the free vector slot. Otherwise, an empty vector will
  /// be appended.
  void push_back(const ScalarSimulationObject& other) {
    // if it was empty or last vector was full, a vector has been added ->
    // update size_ and
    // size_last_vector_
    if (Elements() == 0 || is_full()) {
      size_++;
      size_last_vector_ = 1;
    } else {
      size_last_vector_++;
    }
  }

  /// Gathers scalar elements specified in indexes and stores them in
  /// container `ret`
  /// @param indexes: collection of indexes
  /// @param ret:     scalars are copied to this container
  template <typename T>
  void Gather(const InlineVector<int, 8>& indexes,
              aosoa<T, VcVectorBackend>* ret) const {
    const size_t scalars = indexes.size();
    std::size_t n_vectors = scalars / VcVectorBackend::kVecLen +
                            (scalars % VcVectorBackend::kVecLen ? 1 : 0);
    std::size_t remaining = scalars % VcVectorBackend::kVecLen;
    ret->SetSize(n_vectors);
    for (std::size_t i = 0; i < n_vectors; i++) {
      if (i != n_vectors - 1 || remaining == 0) {
        (*ret)[i].SetSize(VcVectorBackend::kVecLen);
      } else {
        (*ret)[i].SetSize(remaining);
      }
    }

    size_t counter = 0;
    T* dest = nullptr;
    for (size_t i = 0; i < scalars; i++) {
      // TODO(lukas) vectorize the following statements
      int idx = indexes[i];
      size_t src_v_idx = idx / VcVectorBackend::kVecLen;
      size_t src_idx = idx % VcVectorBackend::kVecLen;
      size_t dest_v_idx = counter / VcVectorBackend::kVecLen;
      size_t dest_idx = counter % VcVectorBackend::kVecLen;
      if (dest_idx == 0) {
        dest = &((*ret)[dest_v_idx]);
      }
      dest->CopyFrom(*this, src_v_idx, src_idx, dest_v_idx, dest_idx);
      counter++;
    }
  }

 protected:
  mutable std::size_t idx_ = 0;
  // If Backend is VcSoaBackend use std::size_t, oterwise std::size_t&
  /// Number of vector elements
  typename type_ternary_operator<is_same<Backend, VcSoaBackend>::value,
                                 std::size_t, std::size_t&>::type size_;
  /// Number of scalar elements in the last vector
  typename type_ternary_operator<is_same<Backend, VcSoaBackend>::value,
                                 std::size_t, std::size_t&>::type
      size_last_vector_;
};

/// Helper type trait to map backends to simulation object implementation
template <typename TBackend>
struct SimulationObjectImpl {};

template <>
struct SimulationObjectImpl<VcSoaBackend> {
  typedef SoaSimulationObject<VcSoaBackend> type;
};

template <>
struct SimulationObjectImpl<VcSoaRefBackend> {
  typedef SoaSimulationObject<VcSoaRefBackend> type;
};

template <>
struct SimulationObjectImpl<VcVectorBackend> {
  typedef VectorSimulationObject type;
};

template <>
struct SimulationObjectImpl<ScalarBackend> {
  typedef ScalarSimulationObject type;
};

/// Base simulation object containing methods that are common to all different
/// backends
/// Based on the specified backend, it subclasses the specific implemention
/// (ScalarSimulationObject, VectorSimulationObject or SoaSimulationObject) with
/// the help of type trait SimulationObjectImpl.
BDM_DEFAULT_TEMPLATE(TMemberSelector, TBackend)
struct SimulationObject : public SimulationObjectImpl<TBackend>::type {
 public:
  using Base = typename SimulationObjectImpl<TBackend>::type;

  virtual ~SimulationObject() {}

  // FIXME add for all types of backends??
  template <typename T>
  void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
              std::size_t dest_v_idx, std::size_t dest_idx, T* dest) const {}

  template <typename T>
  void CopyFrom(const T& src, std::size_t src_v_idx, std::size_t src_idx,
                std::size_t dest_v_idx, std::size_t dest_idx) {}

  template <typename Type, typename EnclosingClass, int id>
  using MemberSelector = TMemberSelector<Type, EnclosingClass, id>;

 protected:
  /// Used internally to create the same object, but with
  /// different backend - required since inheritance chain is not known
  /// inside a mixin.
  template <typename TTBackend>
  using Self = SimulationObject<TMemberSelector, TTBackend>;

  /// Used internally to create the same object, but with
  /// different Backend and MemberSelector.
  /// Required since inheritance chain is not known inside a mixin.
  /// Duplication, because compiler g++-4.8 and clang-3.9 did not
  /// accept default value for template template parameter like:
  ///
  ///      template <typename TTBackend = Backend,
  ///                template <typename, typename, int>
  ///                  class TTMemberSelector =
  ///                    Base::template MemberSelector>
  template <typename TTBackend,
            template <typename, typename, int> class TTMemberSelector>
  using Self1 = SimulationObject<TTMemberSelector, TTBackend>;

  using Backend = TBackend;

  SimulationObject() {}

  /// Constructor to create SoaRefBackend
  template <typename T = Backend>
  SimulationObject(
      Self<VcSoaBackend>* other,
      typename enable_if<is_same<T, VcSoaRefBackend>::value>::type* = 0)
      : Base(other) {}

  /// assigment operator if two objects are of the exact same type
  SimulationObject<TMemberSelector, TBackend>& operator=(
      const SimulationObject<TMemberSelector, TBackend>& other) const {
    return *this;
  }

  /// Helper function needed to form a uniform interface to copy scalar values
  /// of data members with different types
  /// destination is NOT a `Container<std::array<vector, ...>>`
  template <typename T, typename U>
  static typename std::enable_if<!is_std_array<typename std::remove_reference<
      decltype(std::declval<T>()[0])>::type>::value>::type
  CopyUtil(T* dest, std::size_t dest_v_idx, std::size_t dest_idx, const U& src,
           std::size_t src_v_idx, std::size_t src_idx) {
    (*dest)[dest_v_idx][dest_idx] = src[src_v_idx][src_idx];
  }

  /// Helper function needed to form a uniform interface to copy scalar values
  /// of data members with different types
  /// destination is a `Container<std::array<vector, ...>>`
  template <typename T, typename U>
  static typename std::enable_if<is_std_array<typename std::remove_reference<
      decltype(std::declval<T>()[0])>::type>::value>::type
  CopyUtil(T* dest, std::size_t dest_v_idx, std::size_t dest_idx, const U& src,
           std::size_t src_v_idx, std::size_t src_idx) {
    for (std::size_t i = 0; i < (*dest)[0].size(); i++) {
      (*dest)[dest_v_idx][i][dest_idx] = src[src_v_idx][i][src_idx];
    }
  }

  /// Helper function needed to form a uniform interface to copy scalar values
  /// of data members with different types
  /// destination is NOT a `std::array<vector, ...>`
  template <typename T, typename U>
  static typename std::enable_if<!is_std_array<typename std::remove_reference<
      decltype(std::declval<U>()[0])>::type>::value>::type
  CopyUtil(T* dest, std::size_t dest_idx, const U& src, std::size_t src_v_idx,
           std::size_t src_idx) {
    (*dest)[dest_idx] = src[src_v_idx][src_idx];
  }

  /// Helper function needed to form a uniform interface to copy scalar values
  /// of data members with different types
  /// destination is a `std::array<vector, ...>`
  template <typename T, typename U>
  static typename std::enable_if<is_std_array<typename std::remove_reference<
      decltype(std::declval<U>()[0])>::type>::value>::type
  CopyUtil(T* dest, std::size_t dest_idx, const U& src, std::size_t src_v_idx,
           std::size_t src_idx) {
    for (std::size_t i = 0; i < (*dest).size(); i++) {
      (*dest)[i][dest_idx] = src[src_v_idx][i][src_idx];
    }
  }
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
