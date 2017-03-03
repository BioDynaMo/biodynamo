#ifndef SIMULATION_OBJECT_H_
#define SIMULATION_OBJECT_H_

#include <type_traits>

#include "backend.h"
#include "inline_vector.h"
#include "aosoa.h"
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

class BdmSimObjectScalarBackend {
public:
  virtual ~BdmSimObjectScalarBackend() {}

  std::size_t size() const {
    return 1;
  }

  bool is_full() const {
    return true;
  }
};

class BdmSimObjectVectorBackend {
public:
  using Backend = VcBackend;

  virtual ~BdmSimObjectVectorBackend() {}

  void SetSize(std::size_t size) { size_ = size; }

  std::size_t size() const {  // FIXME semantic of size here
    return size_;
  }

  std::size_t elements() const {
    return size_;
  }

  bool is_full() const {
    return size_ == Backend::kVecLen;
  }

  void push_back(const BdmSimObjectScalarBackend& other) {
    size_++;
  }


protected:
 std::size_t size_ = Backend::kVecLen;
};

class BdmSimObjectSoaTypeBackend {
  // TODO  order
  // template <typename Type, typename EnclosingClass, int id>
  // using TMemberSelector = TTMemberSelector<Type, EnclosingClass, id>;

  // template <typename TTBackend>
  // using Self = BdmSimObjectSoaTypeBackend<TTMemberSelector, TTBackend>;

  using Backend = VcSoaBackend;

 public:

 virtual ~BdmSimObjectSoaTypeBackend() {}

 void SetSize(std::size_t size) { size_last_vector_ = size; }

  size_t size() const {
    return size_;  // FIXME which size should be returned??
  }

  bool is_full() const {
    return size_last_vector_ == VcBackend::kVecLen;
  }

  // void push_back(const Self<VcBackend>& other) {
  // template <typename T = Backend, typename T1>
  // typename std::enable_if<is_same<T1, VcBackend>::value>::type
  // template <typename T>
  void push_back(const BdmSimObjectVectorBackend& other) {  // FIXME
    size_++;
  }

  // equivalent to std::vector<> clear - it removes all all elements from all
  // data members
  void clear() {
    size_ = 0;
    size_last_vector_ = 0;
  }

  // equivalent to std::vector<> reserve - it increases the capacity
  // of all data member containers
  void reserve(std::size_t new_capacity) {}

  /// \brief returns the number of SOA elements in this container
  std::size_t vectors() const { return size(); }

  /// this function assumes that only the last vector may not be fully
  /// initialized
  std::size_t elements() const {
    if (vectors() != 0) {
      return (vectors() - 1) * Backend::kVecLen + size_last_vector_;
    } else {
      return 0;
    }
  }

  // TODO add documentation
  //push_back scalar on soa
  void push_back(const BdmSimObjectScalarBackend& other) {
    // if it was empty or last vector was full, a vector has been added -> update size_ and
    // size_last_vector_
    if (elements() == 0 || is_full()) {
      size_++;
      size_last_vector_ = 1;
    } else {
      size_last_vector_++;
    }
  }

  // template <typename T>
  virtual void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
              std::size_t dest_v_idx,
              std::size_t dest_idx,
              BdmSimObjectVectorBackend* dest) const {}

  template <typename T>
  void Gather(const InlineVector<int, 8>& indexes,
         aosoa<T, VcBackend>* ret) const {
    const size_t scalars = indexes.size();
    std::size_t n_vectors =
        scalars / Backend::kVecLen + (scalars % Backend::kVecLen ? 1 : 0);
    std::size_t remaining = scalars % Backend::kVecLen;

    ret->SetSize(n_vectors);
    for (std::size_t i = 0; i < n_vectors; i++) {
      if (i != n_vectors - 1 || remaining == 0) {
        (*ret)[i].SetSize(Backend::kVecLen);
      } else {
        (*ret)[i].SetSize(remaining);
      }
    }

    size_t counter = 0;
    BdmSimObjectVectorBackend* dest = nullptr;
    for (size_t i = 0; i < scalars; i++) {
      int idx = indexes[i];
      size_t src_v_idx = idx / Backend::kVecLen;
      size_t src_idx = idx % Backend::kVecLen;
      size_t dest_v_idx = counter / Backend::kVecLen;
      size_t dest_idx = counter % Backend::kVecLen;
      if (dest_idx == 0) {
        dest = &((*ret)[dest_v_idx]);
      }
      CopyTo(src_v_idx, src_idx, dest_v_idx, dest_idx, dest);
      counter++;
    }
  }
protected:
  std::size_t size_ = 1;
  std::size_t size_last_vector_ = VcBackend::kVecLen;
};

template <typename TBackend>
struct BdmSimObjectImpl {};

template <>
struct BdmSimObjectImpl<VcSoaBackend> {
  typedef BdmSimObjectSoaTypeBackend type;
};

template <>
struct BdmSimObjectImpl<VcSoaRefBackend> {
  typedef BdmSimObjectSoaTypeBackend type;
};

template <>
struct BdmSimObjectImpl<VcBackend> {
  typedef BdmSimObjectVectorBackend type;
};

template <>
struct BdmSimObjectImpl<ScalarBackend> {
  typedef BdmSimObjectScalarBackend type;
};

// FIXME rename
BDM_DEFAULT_TEMPLATE(TTMemberSelector, TBackend)
struct BdmSimObject : public BdmSimObjectImpl<TBackend>::type {
 public:
  using Base = typename BdmSimObjectImpl<TBackend>::type;

  virtual ~BdmSimObject() {}

  // FIXME add for all types of backends??
  virtual void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
              std::size_t dest_v_idx,
              std::size_t dest_idx,
              BdmSimObjectVectorBackend* dest) const {}

 protected:
  template <typename Type, typename EnclosingClass, int id>
  using TMemberSelector = TTMemberSelector<Type, EnclosingClass, id>;

  template <typename TTBackend>
  using Self = BdmSimObject<TTMemberSelector, TTBackend>;

  using Backend = TBackend;

  // used to access the SIMD array in a soa container
  // for non Soa Backends index_t will be const so it can be optimized out
  // by the compiler
  typename Backend::index_t idx_ = 0;

  BdmSimObject() {}

  // Ctor to create SoaRefBackend
  // only compiled if T == VcSoaRefBackend
  // template parameter required for enable_if - otherwise compile error
  template <typename T = Backend>
  BdmSimObject(
      Self<VcSoaBackend>& other,
      typename enable_if<is_same<T, VcSoaRefBackend>::value>::type* = 0) {}

  // assigment operator if two objects are of the exact same type
  BdmSimObject<TTMemberSelector, TBackend>& operator=(
      const BdmSimObject<TTMemberSelector, TBackend>& other) const {
    return *this;
  }
};

}  // namespace bdm

#endif  // SIMULATION_OBJECT_H_
