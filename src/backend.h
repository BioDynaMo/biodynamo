#ifndef BACKEND_H_
#define BACKEND_H_

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "Vc/Vc"
#include "macros.h"

namespace bdm {

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

struct Scalar {
  template <typename T>
  using vec = OneElementArray<T>;
};

struct Soa {
  template <typename T>
  using vec = std::vector<T>;
};

struct SoaRef {
  template <typename T>
  using vec = std::vector<T>&;
};

// struct Cuda {
//   template <typename T>
//   using vec = thrust::device_vector<T>;
// };

}  // namespace bdm

#endif  // BACKEND_H_
