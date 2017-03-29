#ifndef BACKEND_H_
#define BACKEND_H_

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "Vc/Vc"
#include "macros.h"

namespace bdm {

/// \brief Holds a reference to a data member of a SOA simulation objects
/// and forwards all its operator calls
template <typename T>
class SoaRefWrapper {
 public:
  explicit SoaRefWrapper(T& data) : data_(data) {}

  // TODO(lukas) add all operators

  BDM_FORCE_INLINE typename T::value_type& operator[](std::size_t index) {
    return data_[index];
  }

  BDM_FORCE_INLINE const typename T::value_type& operator[](
      std::size_t index) const {
    return data_[index];
  }

  template <typename U>
  BDM_FORCE_INLINE auto operator<=(const U& u) const
      -> decltype(std::declval<typename T::value_type>() <= u) {
    return data_ <= u;
  }

  template <typename U>
  BDM_FORCE_INLINE auto operator<(const U& u) const
      -> decltype(std::declval<typename T::value_type>() < u) {
    return data_ < u;
  }

  template <typename U>
  BDM_FORCE_INLINE SoaRefWrapper<T>& operator+=(const U& u) {
    data_ += u;
    return *this;
  }

  BDM_FORCE_INLINE SoaRefWrapper<T>& operator=(const SoaRefWrapper<T>& other) {
    if (this != &other) {
      data_ = other.data_;
    }
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const SoaRefWrapper<T>& wrapper) {
    out << wrapper.data_;
    return out;
  }

  typename T::iterator begin() { return data_.begin(); }
  typename T::iterator end() { return data_.end(); }

  typename T::const_iterator begin() const { return data_.cbegin(); }
  typename T::const_iterator end() const { return data_.cend(); }

 private:
  T& data_;
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
  explicit OneElementArray(const T& data) : data_(data) {}
  explicit OneElementArray(T&& data) : data_(data) {}
  OneElementArray(std::initializer_list<T> list) : data_(*list.begin()) {}

  std::size_t size() const { return 1; }

  BDM_FORCE_INLINE T& operator[](const size_t idx) { return data_; }

  BDM_FORCE_INLINE const T& operator[](const size_t idx) const { return data_; }

  T* begin() { return &data_; }
  T* end() { return &data_ + 1; }

  const T* begin() const { return &data_; }
  const T* end() const { return &data_ + 1; }

 private:
  T data_;
};

/// \brief Defines types and constants for using Vc SIMD vectors in simulation
/// objects
struct VcVectorBackend {
  typedef Vc::double_v::value_type real_t;
  static const size_t kVecLen = Vc::double_v::Size;
  typedef Vc::double_v real_v;
  typedef Vc::double_v::Mask bool_v;
  template <typename T>
  using SimdArray = std::array<T, kVecLen>;
  template <typename T>
  using Container = OneElementArray<T>;
};

/// \brief Defines types and constants for using Vc SIMD vectors in simulation
/// objects and storing them in a SOA memory layout
struct VcSoaBackend {
  static const size_t kVecLen = VcVectorBackend::kVecLen;
  typedef VcVectorBackend::real_v real_v;
  typedef VcVectorBackend::real_t real_t;
  typedef VcVectorBackend::bool_v bool_v;
  template <typename T>
  using SimdArray = typename VcVectorBackend::template SimdArray<T>;
  template <typename T>
  using Container = std::vector<T, Vc::Allocator<T>>;
};

/// \brief Defines types and constants used to create a reference to a
/// simulation object using a VcSoaBackend
struct VcSoaRefBackend {
  static const size_t kVecLen = VcVectorBackend::kVecLen;
  typedef VcVectorBackend::real_v real_v;
  typedef VcVectorBackend::real_t real_t;
  typedef VcVectorBackend::bool_v bool_v;
  template <typename T>
  using SimdArray = typename VcSoaBackend::template SimdArray<T>;
  template <typename T>
  using Container =
      SoaRefWrapper<typename VcSoaBackend::template Container<T>>;
};

/// \brief Defines types and constants for using scalar data types
///
/// Since client code uses the subscript operator ([]) double can't be used
/// directly. Therefore it is wrapped in a Vc::SimdArray of lenght 1
struct ScalarBackend {
  typedef const std::size_t index_t;
  static const size_t kVecLen = 1;
  typedef Vc::SimdArray<double, kVecLen> real_v;
  typedef double real_t;
  typedef std::array<bool, 1> bool_v;
  template <typename T>
  using SimdArray = OneElementArray<T>;
  template <typename T>
  using Container = OneElementArray<T>;
};

inline typename VcVectorBackend::real_v iif(
    const decltype(std::declval<typename VcVectorBackend::real_v>() <
                   std::declval<typename VcVectorBackend::real_v>())& condition,
    const typename VcVectorBackend::real_v& true_value,
    const typename VcVectorBackend::real_v& false_value) {
  return Vc::iif(condition, true_value, false_value);
}

using DefaultBackend = VcVectorBackend;

}  // namespace bdm

#endif  // BACKEND_H_
