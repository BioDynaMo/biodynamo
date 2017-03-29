#ifndef DAOSOA_H_
#define DAOSOA_H_

#include <type_traits>
#include <vector>

#include "aosoa.h"
#include "backend.h"
#include "inline_vector.h"
#include "macros.h"
#include "type_util.h"

namespace bdm {

/// \brief The purpose of this class is to store simulation objects with a
/// vector backend resulting in a AOSOA memory layout.
///
/// Can be used as a `std::vector`, but has added functionality to push back
/// scalar versions of the simulation object and to gather simulation objects
/// based on a collection of indices.
/// The 'd' in daosoa stands for dynamic, since elements can be added to this
/// collection.
template <typename T>
class daosoa {
 public:
  // vector of type T
  using value_type = T;
  using Backend = typename T::Backend;
  using iterator = typename std::vector<value_type>::iterator;
  using const_iterator = typename std::vector<value_type>::const_iterator;

  daosoa() {}
  explicit daosoa(size_t scalar_elements) {
    data_.reserve(scalar_elements / Backend::kVecLen +
                  (scalar_elements % Backend::kVecLen ? 1 : 0));
  }

  explicit daosoa(const value_type& cell) { data_.push_back(cell); }

  /// Returns the number of vector elements in this container
  size_t Vectors() const { return data_.size(); }

  /// This function returns the number of scalar simulation objects in this
  /// container. Assumes that only the last vector may not be fully
  /// initialized.
  size_t Elements() const {
    if (Vectors() != 0) {
      return (Vectors() - 1) * Backend::kVecLen +
             data_[Vectors() - 1].size();  // fixme Size vectors
    } else {
      return 0;
    }
  }

  /// Adds an element at the back of this collection.
  /// Type of value and `value_type` of this class must be the same.
  template <typename T1>
  typename std::enable_if<std::is_same<value_type, T1>::value>::type push_back(
      const T1& value) {
    data_.push_back(std::move(value));
    // fixme broken: what if i first push_back a scalar element and then
    // a vector one??
  }

  /// Adds a scalar element at the back of this collection.
  /// If the last vector in the collection is not full, the scalar instance will
  /// be copied into the free vector slot. Otherwise, an empty vector will
  /// be appended.
  /// Type of value and `value_type` of this class must NOT be the same.
  template <typename T1>
  typename std::enable_if<is_scalar<T1>::value &&
                          !std::is_same<value_type, T1>::value>::type
  push_back(const T1& value) {
    // this implementation is only allowed if T != T1 && T1 == ScalarBackend
    static_assert(
        is_scalar<T1>::value && !std::is_same<value_type, T1>::value,
        "push_back of a non scalar type on a scalar type is not supported");
    if (data_.size() == 0) {
      value_type v;
      v.SetSize(0);
      data_.emplace_back(std::move(v));
    }
    auto last = &data_[data_.size() - 1];
    if (last->is_full()) {
      value_type v1;
      v1.SetSize(0);
      data_.emplace_back(std::move(v1));
      last = &data_[data_.size() - 1];
    }
    last->push_back(value);
  }

  /// Gathers scalar elements specified in indexes and stores them in
  /// container `ret`
  /// @param indexes: collection of indexes
  /// @param ret:     scalars are copied to this container
  void Gather(const InlineVector<int, 8>& indexes,
              aosoa<T, Backend>* ret) const {
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
    value_type* dest = nullptr;
    for (size_t i = 0; i < scalars; i++) {
      int idx = indexes[i];
      size_t vector_idx = idx / Backend::kVecLen;
      size_t vec_el_idx = idx % Backend::kVecLen;
      size_t dest_idx = counter % Backend::kVecLen;
      if (dest_idx == 0) {
        dest = &((*ret)[counter / Backend::kVecLen]);
      }
      data_[vector_idx].CopyTo(0, vec_el_idx, 0, dest_idx, dest);
      counter++;
    }
  }

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
  std::vector<value_type, Vc::Allocator<value_type> > data_;
};

}  // namespace bdm

#endif  // DAOSOA_H_
