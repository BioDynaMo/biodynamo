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

class ScalarSimulationObject {
 public:
  virtual ~ScalarSimulationObject() {}

  std::size_t size() const { return 1; }

  bool is_full() const { return true; }

 protected:
  static const std::size_t idx_ = 0;
};

class VectorSimulationObject {
 public:
  using Backend = VcVectorBackend;

  virtual ~VectorSimulationObject() {}

  void SetSize(std::size_t size) { size_ = size; }

  std::size_t size() const {  // FIXME semantic of size here
    return size_;
  }

  std::size_t Elements() const { return size_; }

  // TODO documentaiton
  /// \brief returns the number of scalar elements in the current vector
  /// Required to have a consistent way to obtain this information between
  /// vector and soa backends
  std::size_t ElementsCurrentVector() const { return size_; }

  std::size_t Vectors() const { return 1; }

  bool is_full() const { return size_ == Backend::kVecLen; }

  void push_back(const ScalarSimulationObject& other) { size_++; }

 protected:
  static const std::size_t idx_ = 0;
  std::size_t size_ = Backend::kVecLen;
};

template <typename TTBackend>
class SoaSimulationObject {
public:
  using Backend = TTBackend;

   template <typename TBackend>
   friend class SoaSimulationObject;

   template <typename T = Backend>
   SoaSimulationObject(typename enable_if<is_same<T, VcSoaBackend>::value>::type* = 0) :
     size_(1), size_last_vector_(VcVectorBackend::kVecLen) {}

   // Ctor to create SoaRefBackend
   // only compiled if T == VcSoaRefBackend
   // template parameter required for enable_if - otherwise compile error
   template <typename T = Backend>
   SoaSimulationObject(
       SoaSimulationObject<VcSoaBackend>* other,
       typename enable_if<is_same<T, VcSoaRefBackend>::value>::type* = 0) :
       size_(other->size_), size_last_vector_(other->size_last_vector_) { }

  virtual ~SoaSimulationObject() {}

  void SetSize(std::size_t size) { size_last_vector_ = size; }

  size_t size() const {
    return size_;  // FIXME which size should be returned??
  }

  bool is_full() const { return size_last_vector_ == VcVectorBackend::kVecLen; }

  // void push_back(const Self<VcVectorBackend>& other) {
  // template <typename T = Backend, typename T1>
  // typename std::enable_if<is_same<T1, VcVectorBackend>::value>::type
  // template <typename T>
  void push_back(const VectorSimulationObject& other) {  // FIXME
    size_++;
    size_last_vector_ = other.ElementsCurrentVector();
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
  std::size_t Vectors() const { return size(); }

  /// this function assumes that only the last vector may not be fully
  /// initialized
  std::size_t Elements() const {
    if (Vectors() != 0) {
      return (Vectors() - 1) * Backend::kVecLen + size_last_vector_;
    } else {
      return 0;
    }
  }

  /// \brief return the number of scalar elements in the current vector
  /// only the last vector might have empty elements
  std::size_t ElementsCurrentVector() const {
    return idx_ == size_ - 1 ? size_last_vector_ : VcVectorBackend::kVecLen;
  }

  // TODO(lukas) add documentation and move next to other push_back
  // push_back scalar on soa
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

  // template <typename T>
  virtual void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
                      std::size_t dest_v_idx, std::size_t dest_idx,
                      VectorSimulationObject* dest) const {}

  template <typename T>
  void Gather(const InlineVector<int, 8>& indexes,
              aosoa<T, VcVectorBackend>* ret) const {
    // TODO remove commented code
    //std::cout << "gather " << (ret == nullptr) << std::endl;
    const size_t scalars = indexes.size();
    std::size_t n_vectors =
        scalars / VcVectorBackend::kVecLen + (scalars % VcVectorBackend::kVecLen ? 1 : 0);
    std::size_t remaining = scalars % VcVectorBackend::kVecLen;
    // std::cout << typeid(ret).name() << std::endl;
    // std::cout << static_cast<void*>(ret) << std::endl;
    //std::cout << "nvectors " << n_vectors << std::endl;
    ret->SetSize(n_vectors);
    //std::cout << "after setsize" << std::endl;
    for (std::size_t i = 0; i < n_vectors; i++) {
      if (i != n_vectors - 1 || remaining == 0) {
        (*ret)[i].SetSize(VcVectorBackend::kVecLen);
      } else {
        (*ret)[i].SetSize(remaining);
      }
    }
    //std::cout << "2" << std::endl;

    size_t counter = 0;
    VectorSimulationObject* dest = nullptr;
    for (size_t i = 0; i < scalars; i++) {
      int idx = indexes[i];
      size_t src_v_idx = idx / VcVectorBackend::kVecLen;
      size_t src_idx = idx % VcVectorBackend::kVecLen;
      size_t dest_v_idx = counter / VcVectorBackend::kVecLen;
      size_t dest_idx = counter % VcVectorBackend::kVecLen;
      if (dest_idx == 0) {
        dest = &((*ret)[dest_v_idx]);
      }
      //std::cout << src_v_idx << " " <<  src_idx << " " << dest_v_idx << " " <<  dest_idx << " - " << (dest == nullptr) << std::endl;
      CopyTo(src_v_idx, src_idx, dest_v_idx, dest_idx, dest);
      counter++;
    }
  }

 protected:
  mutable std::size_t idx_ = 0;
  typename type_ternary_operator<is_same<Backend, VcSoaBackend>::value, std::size_t, std::size_t&>::type size_;
  typename type_ternary_operator<is_same<Backend, VcSoaBackend>::value, std::size_t, std::size_t&>::type size_last_vector_;
};

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

BDM_DEFAULT_TEMPLATE(TTMemberSelector, TBackend)
struct SimulationObject : public SimulationObjectImpl<TBackend>::type {
 public:
  using Base = typename SimulationObjectImpl<TBackend>::type;

  virtual ~SimulationObject() {}

  // FIXME add for all types of backends??
  virtual void CopyTo(std::size_t src_v_idx, std::size_t src_idx,
                      std::size_t dest_v_idx, std::size_t dest_idx,
                      VectorSimulationObject* dest) const {}

 protected:
  template <typename Type, typename EnclosingClass, int id>
  using TMemberSelector = TTMemberSelector<Type, EnclosingClass, id>;

  template <typename TTBackend>
  using Self = SimulationObject<TTMemberSelector, TTBackend>;

  using Backend = TBackend;

  // used to access the SIMD array in a soa container
  // for non Soa Backends index_t will be const so it can be optimized out
  // by the compiler
  // typename Backend::index_t idx_ = 0; // TODO
  // typename Backend::index_t idx_ = 0;
  // TODO document why pointer (otherwise const subscript operator not realiseable)
  // would have different API than AOSOA
  // std::size_t* const idx_ = new std::size_t;   // TODO memleak


  // SimulationObject() { *idx_ = 0; }
  SimulationObject() {}

  // Ctor to create SoaRefBackend
  // only compiled if T == VcSoaRefBackend
  // template parameter required for enable_if - otherwise compile error
  template <typename T = Backend>
  SimulationObject(
      Self<VcSoaBackend>* other,
      typename enable_if<is_same<T, VcSoaRefBackend>::value>::type* = 0) : Base(other) { }

  // assigment operator if two objects are of the exact same type
  SimulationObject<TTMemberSelector, TBackend>& operator=(
      const SimulationObject<TTMemberSelector, TBackend>& other) const {
    return *this;
  }

  template <typename T, typename U>
  static typename std::enable_if<!is_std_array<typename std::remove_reference<
      decltype(std::declval<T>()[0])>::type>::value>::type
  CopyUtil(T* dest, std::size_t dest_v_idx, std::size_t dest_idx, const U& src,
           std::size_t src_v_idx, std::size_t src_idx) {
    (*dest)[dest_v_idx][dest_idx] = src[src_v_idx][src_idx];
  }

  template <typename T, typename U>
  static typename std::enable_if<is_std_array<typename std::remove_reference<
      decltype(std::declval<T>()[0])>::type>::value>::type
  CopyUtil(T* dest, std::size_t dest_v_idx, std::size_t dest_idx, const U& src,
           std::size_t src_v_idx, std::size_t src_idx) {
    for (std::size_t i = 0; i < (*dest)[0].size(); i++) {
      (*dest)[dest_v_idx][i][dest_idx] = src[src_v_idx][i][src_idx];
    }
  }

  template <typename T, typename U>
  static typename std::enable_if<!is_std_array<typename std::remove_reference<
      decltype(std::declval<U>()[0])>::type>::value>::type
  CopyUtil(T* dest, std::size_t dest_idx, const U& src, std::size_t src_v_idx,
           std::size_t src_idx) {
    (*dest)[dest_idx] = src[src_v_idx][src_idx];
  }

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
