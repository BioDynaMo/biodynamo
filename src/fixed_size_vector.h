#ifndef FIXED_SIZE_VECTOR_H_
#define FIXED_SIZE_VECTOR_H_

#include <cstddef>

/// Vector with fixed number of elements == Array with push_back function that
/// keeps track of its size
/// NB: No bounds checking. Do not push_back more often than the number of
/// maximum elements given by the template parameter N
template <typename T, std::size_t N>
class FixedSizeVector {
 public:
  size_t size() const { return size_; }  // NOLINT

  const T& operator[](size_t idx) const { return data_[idx]; }

  T& operator[](size_t idx) { return data_[idx]; }

  FixedSizeVector& operator++() {
    #pragma omp simd
    for (size_t i = 0; i < N; i++) {
      ++data_[i];
    }
    return *this;
  }

  void clear() { size_ = 0; }  // NOLINT

  void push_back(const T& value) { data_[size_++] = value; }  // NOLINT

  const T* begin() const { return &(data_[0]); }    // NOLINT
  const T* end() const { return &(data_[size_]); }  // NOLINT
  T* begin() { return &(data_[0]); }                // NOLINT
  T* end() { return &(data_[size_]); }              // NOLINT

 private:
  T data_[N];
  std::size_t size_ = 0;
};

#endif  // FIXED_SIZE_VECTOR_H_
