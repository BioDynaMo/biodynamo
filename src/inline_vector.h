#ifndef INLINE_VECTOR_H_
#define INLINE_VECTOR_H_

#include <algorithm>
#include <array>
#include <sstream>
#include <vector>

namespace bdm {

/// This containes stores up to N elements without heap allocations
/// If further elements are added elements are stored on the heap
/// Container grows in a geometric sequence
/// Elements are contiguous in memory exept the transition from internal
/// to heap allocated memory (between index N-1 and N)
template <typename T, size_t N>
class InlineVector {
 public:
  InlineVector() {}

  InlineVector(const InlineVector<T, N>& other) {
    data_ = std::move(other.data_);
    size_ = other.size_;
    capacity_ = other.capacity_;
    if (other.heap_data_ != nullptr) {
      heap_data_ = new T[capacity_ - N];
      std::copy_n(other.heap_data_, capacity_ - N, heap_data_);
    }
  }

  InlineVector(InlineVector<T, N>&& other) noexcept {
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;
    heap_data_ = other.heap_data_;
    other.heap_data_ = nullptr;
  }

  virtual ~InlineVector() {
    if (heap_data_ != nullptr) {
      delete[] heap_data_;
    }
  }

  /// Returns the number of elements that the container has currently
  /// allocated space for.
  size_t capacity() const { return capacity_; }

  /// Removes all elements from the container.
  /// Leaves capacity() unchanged.
  void clear() {
    for (size_t i = 0; i < size_; i++) {
      (*this)[i].~T();
    }
    size_ = 0;
  }

  /// Increase the capacity of the container to a value that's greater or equal
  /// to new_capacity. If new_cap is greater than the current `capacity()`,
  /// new storage is allocated, otherwise the method does nothing.
  ///
  /// If new_cap is greater than `capacity()`, all iterators and references,
  /// including the past-the-end iterator, are invalidated.
  /// Otherwise, no iterators or references are invalidated.
  void reserve(size_t new_capacity) {
    if (new_capacity > capacity_) {
      T* new_heap_data = new T[new_capacity];
      if (heap_data_ != nullptr) {
        std::copy_n(heap_data_, capacity_ - N, new_heap_data);
        delete[] heap_data_;
      }
      heap_data_ = new_heap_data;
      capacity_ = new_capacity;
    }
  }

  /// \brief returns the number of elements in this container
  size_t size() const { return size_; }

  /// adds elements to this container and allocates additional memory on the
  /// heap if required
  void push_back(const T& element) {
    if (size_ < N) {
      data_[size_++] = element;
    } else {
      // allocate heap memory
      if (size_ == capacity_) {
        size_t new_capacity = capacity_ * kGrowFactor;
        reserve(new_capacity);
      }
      heap_data_[size_ - N] = element;
      size_++;
    }
  }

  InlineVector<T, N>& operator=(const InlineVector<T, N>& other) {
    if (this != &other) {
      data_ = other.data_;
      size_ = other.size_;
      capacity_ = other.capacity_;
      if (other.heap_data_ != nullptr) {
        if (heap_data_ != nullptr)
          delete[] heap_data_;
        heap_data_ = new T[capacity_ - N];
        std::copy_n(other.heap_data_, capacity_ - N, heap_data_);
      }
    }
    return *this;
  }

  InlineVector<T, N>& operator=(InlineVector<T, N>&& other) {
    if (this != &other) {
      data_ = std::move(other.data_);
      size_ = other.size_;
      capacity_ = other.capacity_;
      heap_data_ = other.heap_data_;
      other.heap_data_ = nullptr;
    }
    return *this;
  }

  T& operator[](size_t index) {
    if (heap_data_ == nullptr) {
      return data_[index];
    } else if (index < N) {
      return data_[index];
    } else {
      return heap_data_[index - N];
    }
  }

  const T& operator[](size_t index) const {
    if (heap_data_ == nullptr) {
      return data_[index];
    } else if (index < N) {
      return data_[index];
    } else {
      return heap_data_[index - N];
    }
  }

  bool operator==(const InlineVector<T, N>& other) const {
    if (size_ != other.size_) {
      return false;
    }
    // inline data
    for (size_t i = 0; i < std::min(size_, N); i++) {
      if (data_[i] != other.data_[i]) {
        return false;
      }
    }
    // heap data
    if (size_ > N) {
      for (size_t i = 0; i < size_ - N; i++) {
        if (heap_data_[i] != other.heap_data_[i]) {
          return false;
        }
      }
    }
    return true;
  }

  friend std::ostream& operator<<(std::ostream& out,
                                  const InlineVector<T, N>& other) {
    for (size_t i = 0; i < other.size_; i++) {
      out << other[i] << ", ";
    }
    return out;
  }

  // TODO(lukas) begin, end, emplace_back, ...

 private:
  static constexpr float kGrowFactor = 1.5;
  std::array<T, N> data_;
  T* heap_data_ = nullptr;
  size_t size_ = 0;
  size_t capacity_ = N;
};

}  // namespace bdm

#endif  // INLINE_VECTOR_H_
