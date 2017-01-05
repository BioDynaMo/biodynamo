#ifndef INLINE_VECTOR_H_
#define INLINE_VECTOR_H_

#include <array>
#include <algorithm>
#include <vector>

namespace bdm {

/// This containes stores up to N elements without heap allocations
/// If further elements are added elements are stored on the heap
/// grows in a geometric sequence
/// elements are contiguous in memory exept the transition from internal
/// to heap allocated memory (between index N-1 and N)
template <typename T, size_t N>
class InlineVector {
 public:
  InlineVector() {}
  virtual ~InlineVector() {
    if (heap_data_ != nullptr) {
      delete[] heap_data_;
    }
  }

  /// \brief returns the number of elements in this container
  size_t size() const { return elements_; }

  /// adds elements to this container and allocates additional memory on the
  /// heap if required
  void push_back(const T& element) {
    if (elements_ < N) {
      data_[elements_++] = element;
    } else {
      // allocate heap memory
      if (elements_ == allocated_elements_) {
        size_t new_allocated_elements = allocated_elements_ * kGrowFactor;
        T* new_heap_data = new T[new_allocated_elements];
        if (heap_data_ != nullptr) {
          std::copy_n(heap_data_, allocated_elements_ - N, new_heap_data);
          delete[] heap_data_;
        }
        heap_data_ = new_heap_data;
        allocated_elements_ = new_allocated_elements;
      }
      heap_data_[elements_ - N] = element;
      elements_++;
    }
  }

  T& operator[](size_t index) {
    if (index < N) {
      return data_[index];
    } else {
      return heap_data_[index - N];
    }
  }

  const T& operator[](size_t index) const {
    if (index < N) {
      return data_[index];
    } else {
      return heap_data_[index - N];
    }
  }

  bool operator==(const InlineVector<T, N>& other) const {
    if (elements_ != other.elements_) {
      return false;
    }
    if (data_ != other.data_) {
      return false;
    }
    for (size_t i = 0; i < elements_ - N; i++) {
      if (heap_data_[i] != other.heap_data_[i]) {
        return false;
      }
    }
    return true;
  }

  // TODO begin, end, emplace_back, ...
 private:
  static constexpr float kGrowFactor = 1.5;
  std::array<T, N> data_;
  T* heap_data_ = nullptr;
  size_t elements_ = 0;
  size_t allocated_elements_ = N;
};

}  // namespace bdm

#endif  // INLINE_VECTOR_H_
