#ifndef AOSOA_H_
#define AOSOA_H_

#include <array>
#include <type_traits>

#include "preprocessor.h"

namespace bdm {

// FIXME code duplication with daosoa
template <typename T, typename Backend>
class aosoa {
 public:
  // vector of type T
  static constexpr size_t kMaxSize = 8 / Backend::kVecLen;
  using value_type = T;
  using iterator = typename std::array<value_type, kMaxSize>::iterator;
  using const_iterator =
      typename std::array<value_type, kMaxSize>::const_iterator;

  aosoa() {}

  void SetSize(std::size_t size) { size_ = size; }

  /// \brief returns the number of SOA elements in this container
  size_t vectors() const { return size_; }

  BDM_FORCE_INLINE value_type& operator[](std::size_t index) {
    return data_[index];
  }
  BDM_FORCE_INLINE const value_type& operator[](std::size_t index) const {
    return data_[index];
  }

  iterator begin() { return data_.begin(); }
  iterator end() { return data_.end(); }

  const_iterator begin() const { return data_.cbegin(); }
  const_iterator end() const { return data_.cend(); }

 private:
  size_t size_ = 0;
  std::array<value_type, kMaxSize> data_;
};

}  // namespace bdm

#endif  // AOSOA_H_
