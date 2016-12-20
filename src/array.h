#ifndef ARRAY_H_
#define ARRAY_H_

#include <type_traits>
#include <vector>
#include "backend.h"

namespace bdm {

// FIXME code duplication with daosoa and aosoa
template <typename T, std::size_t N>
class array {
 public:
  array() {}

  void SetSize(std::size_t size) { size_ = size; }

  /// \brief returns the number of SOA elements in this container
  size_t size() const { return size_; }

  Vc_ALWAYS_INLINE T& operator[](std::size_t index) { return data_[index]; }
  Vc_ALWAYS_INLINE const T& operator[](std::size_t index) const {
    return data_[index];
  }

  bool operator==(const bdm::array<T, N>& other) const {
    if (size_ != other.size_) {
      return false;
    }
    for (size_t i = 0; i < size_; i++) {
      if (data_[i] != other.data_[i]) {
        return false;
      }
    }
    return true;
  }

 private:
  size_t size_ = 0;
  std::array<T, N> data_;
};

}  // namespace bdm

#endif  // ARRAY_H_
