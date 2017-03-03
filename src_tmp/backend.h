#ifndef BACKEND_H_
#define BACKEND_H_

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include <Vc/Vc>

namespace bdm {

template <typename T>
class SoaRefWrapper {
 public:
  SoaRefWrapper(T& data) : data_(data) {}

  // TODO add all operators

  Vc_ALWAYS_INLINE typename T::value_type& operator[](std::size_t index) {
    return data_[index];
  }

  Vc_ALWAYS_INLINE const typename T::value_type& operator[](
      std::size_t index) const {
    return data_[index];
  }

  template <typename U>
  Vc_ALWAYS_INLINE auto operator<=(const U& u) const
      -> decltype(std::declval<typename T::value_type>() <= u) {
    return data_ <= u;
  }

  template <typename U>
  Vc_ALWAYS_INLINE auto operator<(const U& u) const
      -> decltype(std::declval<typename T::value_type>() < u) {
    return data_ < u;
  }

  template <typename U>
  Vc_ALWAYS_INLINE SoaRefWrapper<T>& operator+=(const U& u) {
    data_ += u;
    return *this;
  }

  Vc_ALWAYS_INLINE SoaRefWrapper<T>& operator=(const SoaRefWrapper<T>& other) {
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

/// This class represents an array with exactly one element
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
  OneElementArray(const T& data) : data_(data) {}
  OneElementArray(T&& data) : data_(data) {}
  OneElementArray(std::initializer_list<T> list) : data_(*list.begin()) {}

  Vc_ALWAYS_INLINE T& operator[](const size_t idx) { return data_; }

  Vc_ALWAYS_INLINE const T& operator[](const size_t idx) const { return data_; }

  T* begin() { return &data_; }
  T* end() { return &data_ + 1; }

  const T* begin() const { return &data_; }
  const T* end() const { return &data_ + 1; }

 private:
  T data_;
};

struct VcBackend {
  typedef const std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = Vc::double_v::Size;
  typedef Vc::double_v real_v;
  template <typename T>
  using SimdArray = std::array<T, kVecLen>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = OneElementArray<T>;
};

struct VcSoaBackend {
  typedef std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = VcBackend::kVecLen;
  typedef VcBackend::real_v real_v;
  template <typename T>
  using SimdArray = typename VcBackend::template SimdArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = std::vector<T, Allocator>;
};

struct VcSoaRefBackend {
  typedef std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = VcBackend::kVecLen;
  typedef VcBackend::real_v real_v;
  template <typename T>
  using SimdArray = typename VcSoaBackend::template SimdArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container =
      SoaRefWrapper<typename VcSoaBackend::template Container<T, Allocator>>;
};

struct ScalarBackend {
  typedef const std::size_t index_t;
  typedef double real_t;
  static const size_t kVecLen = 1;
  // TODO change to OneElementArray?
  typedef Vc::SimdArray<double, kVecLen> real_v;
  template <typename T>
  using SimdArray = OneElementArray<T>;
  template <typename T, typename Allocator = std::allocator<T>>
  using Container = OneElementArray<T>;
};

inline typename VcBackend::real_v iif(
    const decltype(std::declval<typename VcBackend::real_v>() <
                   std::declval<typename VcBackend::real_v>())& condition,
    const typename VcBackend::real_v& true_value,
    const typename VcBackend::real_v& false_value) {
  return Vc::iif(condition, true_value, false_value);
}

using DefaultBackend = VcBackend;

template <typename T>
struct is_scalar {
  static const bool value = false;
};

template <template <typename> class Container>
struct is_scalar<Container<ScalarBackend> > {
  static const bool value = true;
};

}  // namespace bdm

#endif  // BACKEND_H_
